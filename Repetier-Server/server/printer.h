/*
 Copyright 2012 Roland Littwin (repetier) repetierdev@gmail.com
 Homepage: http://www.repetier.com
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */


#ifndef __Repetier_Server__printer__
#define __Repetier_Server__printer__

#include <iostream>
#include "libconfig.h++"
#include <deque>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "json_spirit_value.h"
#include <boost/cstdint.hpp>
using namespace boost;

#define MAX_HISTORY_SIZE 50

class PrinterSerial;
class PrinterState;
class PrintjobManager;
class Printjob;
class GCode;
class GCodeDataPacket;

class PrinterResponse {
public:
    uint32_t responseId;
    boost::posix_time::ptime time;
    std::string message;
    /** 
     1 : Commands
     2 : ACK responses like ok, wait, temperature
     4 : Other responses
     8 : Non maskable messages
     */
    uint8_t logtype;
    PrinterResponse(const std::string& message,uint32_t id,uint8_t tp);
    std::string getTimeString();
};
class PrinterHistoryLine {
public:
    uint32_t line;
    std::string command;
    PrinterHistoryLine(const std::string& com,int l):command(com),line(l) {}
};

class Printer {
    friend class PrintjobManager;
    libconfig::Config config;
    PrintjobManager *jobManager;
    PrintjobManager *modelManager;
    PrintjobManager *scriptManager;
    volatile bool stopRequested;
    boost::shared_ptr<boost::thread> thread;
    boost::mutex mutex;
    boost::mutex responseMutex;
    boost::mutex sendMutex;
    std::deque<boost::shared_ptr<PrinterResponse> > responses;
    PrinterSerial *serial;
    uint32_t lastResponseId;
    boost::posix_time::ptime lastTemp; ///< Last temp read. Always access with lastTempMutex
    boost::mutex lastTempMutex;
    void run();
    bool extract(const std::string& source,const std::string& ident,std::string &result);
	std::deque<std::string> manualCommands; ///< Buffer of manual commands to send.
	std::deque<std::string> jobCommands; ///< Buffer of commands comming from a job. Not necessaryly the complete job! Job may refill the buffer if it gets empty.
	std::deque<boost::shared_ptr<GCode> > history; ///< Buffer of the last commands send.
	std::deque<boost::shared_ptr<GCode> > resendLines; ///< Lines for which a resend was requested.
	std::deque<int> nackLines; ///< Length of unacknowledged lines send.
    // Communication handline
    bool readyForNextSend; ///< In pingpong mode indicates that ok was received for the last line.
    bool garbageCleared;
    bool ignoreNextOk; ///< Ignore the next ok because resend already indicated receive
    int receiveCacheFill; ///< Bytes send to the printer that are not acknowledged.
    int resendError;
    int errorsReceived;
    boost::posix_time::ptime lastCommandSend;
    int linesSend;
    std::size_t bytesSend;
    bool paused;
    int updateTempEvery;
    
    /** Resend all lines starting with line. Removes all commands stored in
     history until the given line and moves them instead into the resendLines buffer.
     
     @param line first line to resend.
     */
    void resendLine(size_t line);
    /** Trys to send a GCodeDataPacket. Returns true if the cache rules
     allowed sending. If it was successfull, the line is send and also stored 
     in the history.
     @params dp data packet to send.
     @params gc gcode to save in history.
     @returns true on success. */
    bool trySendPacket(GCodeDataPacket *dp,boost::shared_ptr<GCode> &gc);
    void trySendNextLine(); // Send another line if possible
    void close();
    /** If a line contains a host command starting with @ it is handled in
     this function. Most host commands are ignored as they only have a meaning
     for the running host. Others like @pause are executed.
     */
    void manageHostCommand(boost::shared_ptr<GCode> &cmd);
public:
    double xmin,xmax;
    double ymin,ymax;
    double zmin,zmax;
    double homex,homey,homez;
    double speedx,speedy,speedz,speedeExtrude,speedeRetract;
    std::string name;
    std::string slugName;
    
    std::string device;
    int32_t baudrate;
    bool pingpong;
    int32_t cacheSize;
    bool okAfterResend;
    
    int32_t extruderCount;
    bool active;
    
    int binaryProtocol;
    PrinterState *state;
    
    Printer(std::string conf);
    ~Printer();
    
    void updateLastTempMutex();

    /** The serial reader calls this for each line received. */
    void analyseResponse(std::string &resp);
    
    /** Add response string to list of responses. Removes oldest response if the
     list gets too long. Thread safe. */
    void addResponse(const std::string& msg,uint8_t rtype);
    /** Returns a list with responses, where id is greater as the given response id.
     That way a client can keep track of all responses return from the printer.
     Thread safe. 
     @param resId last known response id.
     @param filter filter selecting which response types should be returned.
     @param lastid last response id contained in list or resId if list is empty.
     */
	boost::shared_ptr<std::list<boost::shared_ptr<PrinterResponse> > > getResponsesSince(uint32_t resId,uint8_t filter,uint32_t &lastid);

    bool shouldInjectCommand(const std::string& cmd);
    /** Push a new manual command into the command queue. Thread safe. */
    void injectManualCommand(const std::string& cmd);
    /** Push a new command into the job queue. Thread safe. */
    void injectJobCommand(const std::string& cmd);
    /** Number of job commands stored */
    size_t jobCommandsStored();
    void fillJSONObject(json_spirit::Object &obj);
    void move(double x,double y,double z,double e);
    int getOnlineStatus();
    bool getActive();
    void setActive(bool v);
    void getJobStatus(json_spirit::Object &obj);
    void connectionClosed();
    inline PrintjobManager *getJobManager() {return jobManager;}
    inline PrintjobManager *getModelManager() {return modelManager;}
    inline PrintjobManager *getScriptManager() {return scriptManager;}
    /** Stop previous pause command */
    void stopPause();
    // Public interthread communication methods
    void startThread();
    void stopThread();
};

#endif /* defined(__Repetier_Server__printer__) */
