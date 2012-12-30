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


#ifndef __Repetier_Server__Printjob__
#define __Repetier_Server__Printjob__

#include <iostream>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "json_spirit_value.h"
#include <fstream>
#include <boost/cstdint.hpp>
using namespace boost;

class Printer;
class Printjob {
public:
    enum PrintjobState {startUpload,stored,running,finished,doesNotExist};
    
    Printjob(std::string _file,bool newjob);
    
    inline bool isNotExistent() {return state==doesNotExist;}
    std::string getName();
    inline int getId() {return id;}
    inline size_t getLength() {return length;}
    inline std::string getFilename() {return file;}
    inline void setFilename(std::string fname) {file = fname;}
    inline void setStored() {state = stored;}
    inline void setRunning() {state = running;}
    inline PrintjobState getState() {return state;}
    inline void setLength(size_t l) {length = l;}
    inline void setPos(size_t p) {pos = p;}
private:
    int id;
    std::string file;
    size_t length; ///< Length of the print file
    size_t pos; ///< Send until this position
    PrintjobState state;

};
typedef boost::shared_ptr<Printjob> PrintjobPtr;

/** The PrintjobManager manages a directory full of possible print jobs.
 The files follow a naming convention to force the order of files added.
 Each file has a number_<Name>.<state>
 
 state is u for the time until it is uploaded completely and gets renamed to g
 after upload is complete. At the start all files with .u get deleted as they
 never finished.
 */
class PrintjobManager {
    std::string directory;
    std::list<PrintjobPtr> files;
    int lastid;
    boost::mutex filesMutex;
    PrintjobPtr runningJob;
    std::ifstream jobin;
    PrintjobPtr findByIdInternal(int id);
public:
    PrintjobManager(std::string dir);
    void cleanupUnfinsihed();
    std::string encodeName(int id,std::string name,std::string postfix,bool withDir);
    static std::string decodeNamePart(std::string file);
    static int decodeIdPart(std::string file);
    void fillSJONObject(std::string name,json_spirit::Object &o);
    PrintjobPtr findById(int id);
    PrintjobPtr createNewPrintjob(std::string name);
    void finishPrintjobCreation(PrintjobPtr job,std::string namerep,size_t sz);
    /** Physically removes job from disk */
    void RemovePrintjob(PrintjobPtr job);
    void startJob(int id);
    void killJob(int id);
    /** Kills the current job without removing it from queue. This
     is needed in case the printer gets disconnected. */
    void undoCurrentJob();
    /** This method is the workhorse for the job printing. It gets called
     frequently and makes sure, the job queue is filled enough for a
     undisrupted print. It will always queue up to 100 commands but no more
     then 10 commands for a call. */
    void manageJobs(Printer *p);
};
#endif /* defined(__Repetier_Server__Printjob__) */
