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


#ifndef __Repetier_Server__PrinterState__
#define __Repetier_Server__PrinterState__

#include <iostream>
#include <boost/thread.hpp>
#include "json_spirit_value.h"
#include <boost/cstdint.hpp>
using namespace boost;

struct PrinterTemp {
    uint64_t time;
    double tempSet;
    double tempRead;
    int8_t output;
};
class Printer;
class GCode;
/**
 The PrinterState stores variable values which are
 changed by sending commands or measured by external sensors
 in the firmware.
 The class is thread safe.
 */
class PrinterState {
    Printer *printer;
    boost::mutex mutex; // Used for thread safety
    int activeExtruder;
    //float extruderTemp;
    bool uploading;
    PrinterTemp bed;
    PrinterTemp* extruder;
    double x, y, z, e,emax,f;
    double lastX,lastY,lastZ,lastE;
    double xOffset, yOffset, zOffset, eOffset;
    double lastZPrint;
    bool fanOn;
    int fanVoltage;
    bool powerOn;
    bool relative;
    bool eRelative;
    int debugLevel;
    uint32_t lastline;
    bool hasXHome, hasYHome, hasZHome;
    double printerWidth, printerHeight, printerDepth;
    int tempMonitor;
    bool drawing;
    int layer;
    bool isG1Move;
    bool eChanged;
    double printingTime;
    bool sdcardMounted;
    
    std::string firmware;
    std::string firmwareURL;
    std::string protocol;
    std::string machine;
    int binaryVersion;
    bool isRepetier;
    bool isMarlin;
    int extruderCountSend;
    int speedMultiply;
    int flowMultiply;
    PrinterTemp& getExtruder(int extruderId);
public:
    
    PrinterState(Printer *p);
    ~PrinterState();
    void reset();
    /** Returns the extruder temperature structure. 
     @param extruderId Id of the extruder. -1 for active extruder.
     @returns Temperature state of selected extruder.
     */
    const PrinterTemp& getExtruder(int extruderId) const;
    /** Analyses the gcode and changes the status variables accordingly. */
    void analyze(GCode &code);
    /** Analyse the response */
    void analyseResponse(const std::string &res,uint8_t &rtype);
    bool extract(const std::string& source,const std::string& ident,std::string &result);
    /** Increases the line counter.
     @returns Increased line number. */
    uint32_t increaseLastline();
    uint32_t decreaseLastline();
    uint32_t getLastline() {boost::mutex::scoped_lock l(mutex);return lastline;}
    void fillJSONObject(json_spirit::Object &obj);
    std::string getMoveXCmd(double dx,double f);
    std::string getMoveYCmd(double dy,double f);
    std::string getMoveZCmd(double dz,double f);
    std::string getMoveECmd(double de,double f);
};
#endif /* defined(__Repetier_Server__PrinterState__) */
