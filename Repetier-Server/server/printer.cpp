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

#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005
#define _CRT_SECURE_NO_DEPRECATE 
#define _SCL_SECURE_NO_DEPRECATE 

#include "printer.h"
#include "Printjob.h"
#include "GCode.h"
#include "PrinterSerial.h"
#include "PrinterState.h"
#include "global_config.h"
#include <boost/filesystem.hpp>
#include "json_spirit.h"
#include "RLog.h"

using namespace std;
using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost;

PrinterResponse::PrinterResponse(const string& mes,uint32_t id,uint8_t tp):message(mes) {
    responseId = id;
    logtype = tp;
    time = boost::posix_time::microsec_clock::local_time();
}

std::string PrinterResponse::getTimeString() {
    tm tm = boost::posix_time::to_tm(time);
    char buf[40];
    sprintf(buf,"%2d:%02d:%02d",tm.tm_hour,tm.tm_min,tm.tm_sec);
    return string(buf);
}
Printer::Printer(string conf) {
    stopRequested = false;
    okAfterResend = true;
    try {
        config.readFile(conf.c_str());
        bool ok = true;
        ok &= config.lookupValue("printer.name", name);
        ok &= config.lookupValue("printer.slugName", slugName);
        ok &= config.lookupValue("printer.connection.device", device);
        ok &= config.lookupValue("printer.connection.baudrate", baudrate);
        ok &= config.lookupValue("printer.connection.pingPong", pingpong);
        ok &= config.lookupValue("printer.connection.readCacheSize", cacheSize);
        ok &= config.lookupValue("printer.connection.protocol", binaryProtocol);
        ok &= config.lookupValue("printer.connection.okAfterResend", okAfterResend);
        ok &= config.lookupValue("printer.dimension.xmin", xmin);
        ok &= config.lookupValue("printer.dimension.ymin", ymin);
        ok &= config.lookupValue("printer.dimension.zmin", zmin);
        ok &= config.lookupValue("printer.dimension.xmax", xmax);
        ok &= config.lookupValue("printer.dimension.ymax", ymax);
        ok &= config.lookupValue("printer.dimension.zmax", zmax);
        ok &= config.lookupValue("printer.homing.xhome", homex);
        ok &= config.lookupValue("printer.homing.yhome", homey);
        ok &= config.lookupValue("printer.homing.zhome", homez);
        ok &= config.lookupValue("printer.extruder.count", extruderCount);
        ok &= config.lookupValue("printer.extruder.tempUpdateEvery",updateTempEvery);
        ok &= config.lookupValue("active", active);
        ok &= config.lookupValue("printer.speed.xaxis", speedx);
        ok &= config.lookupValue("printer.speed.yaxis", speedy);
        ok &= config.lookupValue("printer.speed.zaxis", speedz);
        ok &= config.lookupValue("printer.speed.eaxisExtrude", speedeExtrude);
        ok &= config.lookupValue("printer.speed.eaxisRetract", speedeRetract);
        
        if(!ok) {
            RLog::log("Printer configuration @ not complete",conf,true);
            exit(4);
        }
        lastResponseId = 0;
        state = new PrinterState(this);
        serial = new PrinterSerial(*this);
        resendError = 0;
        errorsReceived = 0;
        linesSend = 0;
        bytesSend = 0;
        paused = false;
    }
    catch(libconfig::ParseException& pe) {
        RLog::log("Error parsing printer configuration @",conf);
        RLog::log(static_cast<string>(pe.getError())+" line @",pe.getLine());
        exit(4);
    }
    catch(...) {
        RLog::log("Error reading printer configuration @",conf);
        exit(4);
    }
#ifdef DEBUG
    cout << "Printer configuration read: " << name << endl;
    cout << "Port:" << device << endl;
#endif
    jobManager = new PrintjobManager(gconfig->getStorageDirectory()+slugName+"/"+"jobs");
    modelManager = new PrintjobManager(gconfig->getStorageDirectory()+slugName+"/"+"models");
}
Printer::~Printer() {
    serial->close();
    delete state;
    delete serial;
    delete modelManager;
    delete jobManager;
}
void Printer::startThread() {
    assert(!thread);
    thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Printer::run, this)));
    
}
void Printer::updateLastTempMutex() {
    mutex::scoped_lock l(lastTempMutex);
    lastTemp = boost::posix_time::microsec_clock::local_time();
}

void Printer::run() {
    while (!stopRequested)
    {
        try {
            boost::this_thread::sleep(boost::posix_time::milliseconds(10));
            if(!active) {
                if(serial->isConnected())
                    serial->close();
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                continue; // skip normal usage
            }
            if(!serial->isConnected()) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                serial->tryConnect();
            } else {
                {
                    time_duration td;
                    {
                        mutex::scoped_lock l(lastTempMutex);
                        posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
                        td = now-lastTemp;
                    } // Must close mutex to prevent deadlock!
                    if(manualCommands.size()<5 && updateTempEvery>0 && td.seconds()>=updateTempEvery) {
                        injectManualCommand("M105");
                        lastTemp = microsec_clock::local_time();
                    }
                }
                jobManager->manageJobs(this); // refill job queue
            }
            trySendNextLine();
        }
        catch( boost::thread_interrupted) {
            stopRequested = true;
        }
    }
}
void Printer::stopThread() {
    thread->interrupt();
    thread->join();
#ifdef DEBUG
    cout << "Thread for printer " << name << " finished" << endl;
#endif
}
void Printer::connectionClosed() {
    jobManager->undoCurrentJob();
    mutex::scoped_lock l(sendMutex);
    manualCommands.clear();
}

void Printer::addResponse(const std::string& msg,uint8_t rtype) {
    mutex::scoped_lock l(responseMutex);
    shared_ptr<PrinterResponse> newres(new PrinterResponse(msg,++lastResponseId,rtype));
    responses.push_back(newres);
    if(responses.size()>(size_t)gconfig->getBacklogSize())
        responses.pop_front();
}
void Printer::injectManualCommand(const std::string& cmd) {
    if(cmd=="@kill") {
        serial->resetPrinter();
        return;
    }
    if(cmd.length()==0) return; // Don't waste time with empty lines
    {
        mutex::scoped_lock l(sendMutex);
        manualCommands.push_back(cmd);
        //RLog::log(cmd+" injected",(int)manualCommands.size());
    } // need parantheses to prevent deadlock with trySendNextLine
    trySendNextLine(); // Check if we need to send the command immediately
}
void Printer::injectJobCommand(const std::string& cmd) {
    mutex::scoped_lock l(sendMutex);
    jobCommands.push_back(cmd);
    // No need to trigger job commands early. There will most probably follow more very soon
    // and the job should already run.
}
void Printer::move(double x,double y,double z,double e) {
    if(x!=0)
        injectManualCommand(state->getMoveXCmd(x, speedx*60.0));
    if(y!=0)
        injectManualCommand(state->getMoveYCmd(y, speedx*60.0));
    if(z!=0)
        injectManualCommand(state->getMoveZCmd(z, speedx*60.0));
    if(e!=0)
        injectManualCommand(state->getMoveECmd(e,60.0 * (e>0 ? speedeExtrude : speedeRetract)));
}

size_t Printer::jobCommandsStored() {
    mutex::scoped_lock l(sendMutex);
    return jobCommands.size();
}

boost::shared_ptr<list<boost::shared_ptr<PrinterResponse> > > Printer::getResponsesSince(uint32_t resId,uint8_t filter,uint32_t &lastid) {
    lastid = resId;
    shared_ptr<list<shared_ptr<PrinterResponse> > > retList(new list<shared_ptr<PrinterResponse> >());
    mutex::scoped_lock l(responseMutex);
    deque<shared_ptr<PrinterResponse> >::iterator act = responses.begin();
    deque<shared_ptr<PrinterResponse> >::iterator e = responses.end();
    while(act!=e) {
        if((*act)->responseId>resId && ((*act)->logtype & filter)!=0) {
            retList->push_back(*act);
            lastid = (*act)->responseId;
        }
        act++;
    }
    return retList;
}
void Printer::close() {
    serial->close();
}
void Printer::resendLine(size_t line)
{
    {
        mutex::scoped_lock l(sendMutex);
        ignoreNextOk = okAfterResend;
        resendError++;
        errorsReceived++;
        if(!pingpong && errorsReceived==3 && cacheSize>63) {
            cacheSize = 63;
        }
        if (pingpong)
            readyForNextSend = true;
        else  {
            nackLines.clear();
            receiveCacheFill = 0;
        }
    
        if (resendError > 8) {
            string msg = "Receiving only error messages. Reset communication.";
            string url = "/printer/msg/"+slugName+"?a=close";
            gconfig->createMessage(msg, url);
            serial->resetPrinter();
        }
        line &=65535;
        resendLines.clear();
        bool addLines = false;
        for(deque<shared_ptr<GCode> >::iterator it=history.begin();it!=history.end();++it) {
            GCode &gc = **it;
            if (gc.hasN() && (gc.getN() & 65535) == line)
                addLines = true;
            if(addLines) resendLines.push_back(*it);
        }
        if (binaryProtocol != 0) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(320000/baudrate));
            uint8_t  buf[32];
            for (int i = 0; i < 32; i++) buf[i] = 0;
            serial->writeBytes(buf,32);
            boost::this_thread::sleep(boost::posix_time::milliseconds(320000/baudrate));
        } else {
            boost::this_thread::sleep(boost::posix_time::milliseconds(cacheSize*10000/baudrate)); // Wait for buffer to empty
        }
    } // unlock mutex or we get deadlock!
    trySendNextLine();
}
void Printer::manageHostCommand(boost::shared_ptr<GCode> &cmd) {
    string c = cmd->hostCommandPart();
    if(c=="@pause") {
        string msg= "Printer "+name+" paused:"+cmd->hostParameter();
        string answer = "/printer/msg/"+slugName+"?a=unpause";
        gconfig->createMessage(msg,answer);
        paused = true;
    } else if(c=="@isathome") {
        state->setIsathome();
    } else if(c=="@kill") {
        serial->resetPrinter();
    }
}
void Printer::stopPause() {
    mutex::scoped_lock l(sendMutex);
    paused = false;
}
bool Printer::trySendPacket(GCodeDataPacket *dp,shared_ptr<GCode> &gc) {
    if((pingpong && readyForNextSend) || (!pingpong && cacheSize>receiveCacheFill+dp->length)) {
        serial->writeBytes(dp->data,dp->length);
        if(!pingpong) {
            receiveCacheFill += dp->length;
            nackLines.push_back(dp->length);
        } else readyForNextSend = false;
        history.push_back(gc);
        if(history.size()>MAX_HISTORY_SIZE)
            history.pop_front();
        lastCommandSend = boost::posix_time::microsec_clock::local_time();
        bytesSend+=dp->length;
        linesSend++;
        addResponse(gc->getOriginal(), 1);
        return true;
    }
    return false;
}
// Extract the value following a identifier ident until the next space or line end.
bool Printer::extract(const string& source,const string& ident,string &result)
{
    size_t pos = 0; source.find(ident);
    do
    {
        if(pos>0) pos++;
        pos = source.find(ident, pos);
        if (pos == string::npos) return false;
        if(pos==0) break;
    } while (source[pos-1]!=' ');
    size_t start = pos + ident.length();
    size_t end = start;
    size_t len = source.length();
    while (end < len && source[end] != ' ') end++;
    result = source.substr(start,end-start);
    return true;
}
void Printer::trySendNextLine() {
    if (!garbageCleared) return;
    mutex::scoped_lock l(sendMutex);
    if (pingpong && !readyForNextSend) {return;}
    if (!serial->isConnected()) {return;} // Not ready yet
    shared_ptr<GCode> gc;
    GCodeDataPacket *dp = NULL;
    // first resolve old communication problems
    if (resendLines.size()>0) {
        gc = resendLines.front();
        if (binaryProtocol == 0 || gc->forceASCII)
            dp = gc->getAscii(true,true);
        else
            dp = gc->getBinary();
        if(trySendPacket(dp,gc))
        {
            //[rhlog addText:[@"Resend: " stringByAppendingString:[gc getAsciiWithLine:YES withChecksum:YES]]];
        }
        return;
    }
    if (resendError > 0) resendError--; // Drop error counter
                                        // then check for manual commands
    if (manualCommands.size() > 0)  {
        gc = shared_ptr<GCode>(new GCode(*this,manualCommands.front()));
        if (gc->hostCommand)
        {
            manageHostCommand(gc);
            manualCommands.pop_front();
            return;
        }
        if(gc->m!=117)
            gc->setN(state->increaseLastline());
        if (binaryProtocol == 0 || gc->forceASCII)
            dp = gc->getAscii(true,true);
        else
            dp = gc->getBinary();
        if(trySendPacket(dp,gc)) {
            manualCommands.pop_front();
            state->analyze(*gc);
        } else if(gc->hasN() && !(gc->hasM() && gc->getM()==110)) state->decreaseLastline();
        return;
    }
    // do we have a printing job?
    if (jobCommands.size()>0 && !paused)  {
        gc = shared_ptr<GCode>(new GCode(*this,jobCommands.front()));
        if (gc->hostCommand)
        {
            manageHostCommand(gc);
            jobCommands.pop_front();
            return;
        }
        if(gc->m!=117)
            gc->setN(state->increaseLastline());
        if (binaryProtocol == 0 || gc->forceASCII)
            dp = gc->getAscii(true,true);
        else
            dp = gc->getBinary();
        if(trySendPacket(dp,gc)) {
            jobCommands.pop_front();
            state->analyze(*gc);
        } else if(gc->hasN() && !(gc->hasM() && gc->getM()==110)) state->decreaseLastline();
        return;
    }
}
void Printer::analyseResponse(string &res) {
#ifdef DEBUG
    //   cout << "Response:" << res << endl;
#endif
    uint8_t rtype = 4;
    while(res.length()>0 && res[0]<32)
        res = res.substr(1);
    string h;
    
    state->analyseResponse(res,rtype); // Update state variables
    size_t fpos = res.find("start");
    if (fpos==0 ||
        (garbageCleared==false && fpos!=string::npos))
    {
        {
            mutex::scoped_lock l(sendMutex);
            state->reset();
            // [job killJob]; // continuing the old job makes no sense, better save the plastic
            history.clear();
            readyForNextSend = true;
            nackLines.clear();
            receiveCacheFill = 0;
            garbageCleared = true;
            manualCommands.clear();
            jobManager->undoCurrentJob();
        }
        injectManualCommand("M110 N0");
        injectManualCommand("M115");
   }
    if (extract(res,"Resend:",h))
    {
        size_t line = atoi(h.c_str());
        resendLine(line);
    }
    else if (res.find("ok")==0)
    {
        rtype = 2;
        garbageCleared = true;
        //if(Main.main.logView.toolACK.Checked)
        //    log(res, true, level);
        if (!ignoreNextOk)  // ok in response of resend?
        {
            if (pingpong) readyForNextSend = true;
            else
            {
                mutex::scoped_lock l(sendMutex);
                if (nackLines.size() > 0) {
                    receiveCacheFill-= nackLines.front();
                    nackLines.pop_front();
                }
            }
            resendError = 0;
            trySendNextLine();
        } else
            ignoreNextOk = false;
    }
    else if (res=="wait") 
    {
        rtype = 2;
        mutex::scoped_lock l(sendMutex);
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        time_duration td(now-lastCommandSend);
        if(td.total_seconds()>5) {
            if (pingpong) readyForNextSend = true;
            else
            {
                nackLines.clear();
                receiveCacheFill = 0;
            }
        }
        resendError = 0;
    }
    addResponse(res,rtype);
    trySendNextLine();
}

int Printer::getOnlineStatus() {
    if(serial->isConnected()) return 1;
    return 0;
}
bool Printer::getActive() {
    return active;
}
void Printer::setActive(bool v) {
    active = v;
}
void Printer::getJobStatus(json_spirit::Object &obj) {
    jobManager->getJobStatus(obj);
}
void Printer::fillJSONObject(json_spirit::Object &obj) {
    using namespace json_spirit;
    obj.push_back(Pair("active",active));
    obj.push_back(Pair("paused",paused));
    obj.push_back(Pair("printerName",name));
    obj.push_back(Pair("slug",slugName));
    obj.push_back(Pair("device",device));
    obj.push_back(Pair("baudrate",baudrate));
    obj.push_back(Pair("xmin",xmin));
    obj.push_back(Pair("xmax",xmax));
    obj.push_back(Pair("ymin",ymin));
    obj.push_back(Pair("ymax",ymax));
    obj.push_back(Pair("zmin",zmin));
    obj.push_back(Pair("zmax",zmax));
    obj.push_back(Pair("speedx",speedx));
    obj.push_back(Pair("speedy",speedy));
    obj.push_back(Pair("speedz",speedz));
    obj.push_back(Pair("speedeExtrude",speedeExtrude));
    obj.push_back(Pair("speedeRetract",speedeRetract));
    obj.push_back(Pair("extruderCount",extruderCount));
    Array ea;
    for(int i=0;i<extruderCount;i++) {
        Object e;
        e.push_back(Pair("extruderid",i));
        e.push_back(Pair("extrudernum",i+1));
        ea.push_back(e);
    }
    obj.push_back(Pair("extruder",ea));
}
