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

#include "PrinterState.h"
#include "printer.h"
#include "GCode.h"

using namespace std;
using namespace boost;
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005
#endif

PrinterState::PrinterState(Printer *p) {
    printer = p;
    extruder=new PrinterTemp[printer->extruderCount+1]; // Always one more in case 0 extruder
    reset();
}
    
void PrinterState::reset() {
    mutex::scoped_lock l(mutex);
    activeExtruder = 0;
    uploading = false;
    bed.output = 0;
    bed.tempSet = bed.tempRead = 0;
    x = y = z = e = emax = 0;
    f = 1000;
    lastX = lastY = lastZ = lastE = 0;
    xOffset = yOffset = zOffset = eOffset = 0;
    fanOn = false;
    fanVoltage = 0;
    powerOn = true;
    relative = false;
    eRelative = false;
    debugLevel = 6;
    lastline = 0;
    lastZPrint = 0;
    printingTime = 0;
    layer=0;
    hasXHome = hasYHome = hasZHome = false;
    tempMonitor = -1;
    binaryVersion = 0;
    sdcardMounted = true;
    isRepetier = false;
    isMarlin = false;
    speedMultiply = 100;
    flowMultiply = 100;
}

PrinterState::~PrinterState() {
    delete[] extruder;
}
const PrinterTemp& PrinterState::getExtruder(int extruderId) const {
    if(extruder<0) extruderId = activeExtruder;
    if(extruderId>=printer->extruderCount) extruderId = 0;
    return extruder[extruderId];
}
PrinterTemp& PrinterState::getExtruder(int extruderId) {
    if(extruder<0) extruderId = activeExtruder;
    if(extruderId>=printer->extruderCount) extruderId = 0;
    return extruder[extruderId];
}
void PrinterState::analyze(GCode &code)
{
    mutex::scoped_lock l(mutex);
    isG1Move = false;
    if (code.hostCommand)
    {
        string hc = code.hostCommandPart();
        if (hc=="@isathome")
        {
            hasXHome = hasYHome = hasZHome = true;
            x = printer->homex;
            xOffset = 0;
            y = printer->homey;
            yOffset = 0;
            z = printer->homez;
            zOffset = 0;
        }
        return;
    }
    if (code.hasN())
        lastline = code.getN();
        if (uploading && !code.hasM() && code.getM() != 29) return; // ignore upload commands
    if (code.hasG())
    {
        switch (code.getG())
        {
            case 0:
            case 1:
            {
                eChanged = false;
                if(code.hasF()) f = code.getF();
                    if (relative)
                    {
                        if(code.hasX()) x += code.getX();
                            if(code.hasY()) y += code.getY();
                                if(code.hasZ()) z += code.getZ();
                                    if(code.hasE()) {
                                        eChanged = code.getE()!=0;
                                        e += code.getE();
                                    }
                    }
                    else
                    {
                        if (code.hasX()) x = xOffset+code.getX();
                            if (code.hasY()) y = yOffset+code.getY();
                                if (code.hasZ()) {
                                    z = zOffset+code.getZ();
                                }
                        if (code.hasE())
                        {
                            if (eRelative) {
                                eChanged = code.getE()!=0;
                                e += code.getE();
                            } else {
                                eChanged = (eOffset+code.getE())!=e;
                                e = eOffset + code.getE();
                            }
                        }
                    }
                if (x < printer->xmin) { x = printer->xmin; hasXHome = false; }
                if (y < printer->ymin) { y = printer->ymin; hasYHome = false; }
                if (z < printer->zmin) { z = printer->zmin; hasZHome = false; }
                if (x > printer->xmax) { x = printer->xmax; hasXHome = false; }
                if (y > printer->ymax) { y = printer->ymax; hasYHome = false; }
                if (z > printer->zmax) { z = printer->zmax; hasZHome = false; }
                if (e > emax) {
                    emax = e;
                    if(z!=lastZPrint) {
                        lastZPrint = z;
                        layer++;
                    }
                }
                double dx = abs(x - lastX);
                double dy = abs(y - lastY);
                double dz = abs(z - lastZ);
                double de = abs(e - lastE);
                if (dx + dy + dz > 0.001)
                {
                    printingTime += sqrt(dx * dx + dy * dy + dz * dz) * 60.0f / f;
                }
                else printingTime += de * 60.0f / f;
                lastX = x;
                lastY = y;
                lastZ = z;
                lastE = e;
            }
                break;
            case 28:
            case 161:
            {
                bool homeAll = !(code.hasX() || code.hasY() || code.hasZ());
                if (code.hasX() || homeAll) { xOffset = 0; x = printer->homex; hasXHome = true; }
                if (code.hasY() || homeAll) { yOffset = 0; y = printer->homey; hasYHome = true; }
                if (code.hasZ() || homeAll) { zOffset = 0; z = printer->homez; hasZHome = true; }
                if (code.hasE()) { eOffset = 0; e = 0; emax = 0; }
                break;
            }
            case 162:
            {
                bool homeAll = !(code.hasX() || code.hasY() || code.hasZ());
                if (code.hasX() || homeAll) { xOffset = 0; x = printer->xmax; hasXHome = true; }
                if (code.hasY() || homeAll) { yOffset = 0; y = printer->ymax; hasYHome = true; }
                if (code.hasZ() || homeAll) { zOffset = 0; z = printer->zmax; hasZHome = true; }
                break;
            }
            case 90:
                relative = false;
                break;
            case 91:
                relative = true;
                break;
            case 92:
                if (code.hasX()) { xOffset = x-code.getX(); x = xOffset; }
                if (code.hasY()) { yOffset = y-code.getY(); y = yOffset; }
                if (code.hasZ()) { zOffset = z-code.getZ(); z = zOffset; }
                if (code.hasE()) { eOffset = e-code.getE(); lastE = e = eOffset; }
                break;
        }
    }
    else if (code.hasM())
    {
        switch (code.getM())
        {
            case 28:
                uploading = true;
                break;
            case 29:
                uploading = false;
                break;
            case 80:
                powerOn = true;
                break;
            case 81:
                powerOn = false;
                break;
            case 82:
                eRelative = false;
                break;
            case 83:
                eRelative = true;
                break;
            case 104:
            case 109:
            {
                int t = -1;
                if(code.hasT()) t = code.getT();
                    if (code.hasS()) getExtruder(t).tempSet = code.getS();
            }
                break;
            case 106:
                fanOn = true;
                if (code.hasS()) fanVoltage = code.getS();
                break;
            case 107:
                fanOn = false;
                break;
            case 110:
                lastline = code.getN();
                break;
            case 111:
                if (code.hasS())
                {
                    debugLevel = code.getS();
                }
                break;
            case 140:
            case 190:
                if (code.hasS()) bed.tempSet = code.getS();
                break;
            case 203: // Temp monitor
                tempMonitor = code.getS();
                break;
        }
    }
    else if (code.hasT())
    {
        activeExtruder = code.getT();
    }
}
// Extract the value following a identifier ident until the next space or line end.
bool PrinterState::extract(const string& source,const string& ident,string &result)
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
void PrinterState::analyseResponse(const string &res,uint8_t &rtype) {
    mutex::scoped_lock l(mutex);
    string h;
    char b[100];
    if (extract(res,"FIRMWARE_NAME:",h))
    {
        firmware = h;
        isRepetier = h.find("Repetier")!=string::npos;
        isMarlin = h.find("Marlin")!=string::npos;
        if (extract(res,"FIRMWARE_URL:",h))
        {
            firmwareURL = h;
        }
        if (extract(res,"PROTOCOL_VERSION:",h))
        {
            protocol = h;
        }
        if (extract(res,"MACHINE_TYPE:",h))
        {
            machine = h;
        }
        if (extract(res,"EXTRUDER_COUNT:",h))
        {
            extruderCountSend = atoi(h.c_str());
        }
    }
    if (extract(res,"X:",h))
    {
        double v = atof(h.c_str());
        x = v-xOffset;
    }
    if (extract(res,"Y:",h))
    {
        double v = atof(h.c_str());
        y = v-yOffset;
    }
    if (extract(res,"Z:",h))
    {
        double v = atof(h.c_str());
        z = v-zOffset;
    }
    if (extract(res,"E:",h))
    {
        double v = atof(h.c_str());
        e = v;
    }
    if (extract(res,"T0",h)) {
        int ecnt = 0;
        do {
            sprintf(b,"T%d:",ecnt);
            if(!extract(res,b,h)) break;
            double t = atof(h.c_str());
            PrinterTemp &ex = getExtruder(ecnt);
            ex.tempRead = t;
            sprintf(b,"@%d:",ecnt);
            if(extract(res,b,h)) {
                int  eo = atoi(h.c_str());
                if(isMarlin) eo*=2;
                ex.output = eo;
            }
            ecnt++;
        } while(true);
    }
    if (extract(res,"T:",h))
    {
        rtype = 2;
        PrinterTemp &ex = getExtruder(-1);
        ex.tempRead = atof(h.c_str());
        if (extract(res,"@:",h))
        {
            int eo = atoi(h.c_str());
            if(isMarlin) eo*=2;
            ex.output = eo;
        }
        printer->updateLastTempMutex();
    }
    if (extract(res,"B:",h))
    {
        bed.tempRead = atof(h.c_str());
    }
    if (extract(res,"SpeedMultiply:",h))  {
        rtype = 2;
        speedMultiply = atoi(h.c_str());
    }
    if (extract(res,"FlowMultiply:",h))  {
        rtype = 2;
        flowMultiply = atoi(h.c_str());
    }
    if (extract(res,"TargetExtr0:",h))  {
        rtype = 2;
        PrinterTemp &ex = getExtruder(0);
        ex.tempSet = atof(h.c_str());
    }
    if (extract(res,"TargetExtr1:",h))  {
        rtype = 2;
        PrinterTemp &ex = getExtruder(0);
        ex.tempSet = atof(h.c_str());
    }
    if (extract(res,"TargetBed:",h))  {
        rtype = 2;
        bed.tempSet = atof(h.c_str());
    }
    if (extract(res,"Fanspeed:",h))  {
        rtype = 2;
        fanVoltage = atoi(h.c_str());
    }
    if (extract(res,"REPETIER_PROTOCOL:",h))
    {
        binaryVersion = atoi(h.c_str());
    }
}
uint32_t PrinterState::increaseLastline() {
    mutex::scoped_lock l(mutex);
    return ++lastline;
}
uint32_t PrinterState::decreaseLastline() {
    mutex::scoped_lock l(mutex);
    return --lastline;
}
std::string PrinterState::getMoveXCmd(double dx,double f) {
    mutex::scoped_lock l(mutex);
    char buf[100];
    sprintf(buf,"G1 X%.2f F%.0f",relative ? dx : x+dx,f);
    return string(buf);
}
std::string PrinterState::getMoveYCmd(double dy,double f) {
    mutex::scoped_lock l(mutex);
    char buf[100];
    sprintf(buf,"G1 Y%.2f F%.0f",relative ? dy : y+dy,f);
    return string(buf);
    
}
std::string PrinterState::getMoveZCmd(double dz,double f) {
    mutex::scoped_lock l(mutex);
    char buf[100];
    sprintf(buf,"G1 Z%.2f F%.0f",relative ? dz : z+dz,f);
    return string(buf);
    
}
std::string PrinterState::getMoveECmd(double de,double f) {
    mutex::scoped_lock l(mutex);
    char buf[100];
    sprintf(buf,"G1 E%.2f F%.0f",relative || eRelative ? de : e+de,f);
    return string(buf);    
}

void PrinterState::fillJSONObject(json_spirit::Object &obj) {
    using namespace json_spirit;
    mutex::scoped_lock l(mutex);
    obj.push_back(Pair("activeExtruder",activeExtruder));
    obj.push_back(Pair("x",x));
    obj.push_back(Pair("y",y));
    obj.push_back(Pair("z",z));
    obj.push_back(Pair("fanOn",fanOn));
    obj.push_back(Pair("fanVoltage",fanVoltage));
    obj.push_back(Pair("powerOn",powerOn));
    obj.push_back(Pair("debugLevel",debugLevel));
    obj.push_back(Pair("hasXHome",hasXHome));
    obj.push_back(Pair("hasYHome",hasYHome));
    obj.push_back(Pair("hasZHome",hasZHome));
    obj.push_back(Pair("layer",layer));
    obj.push_back(Pair("sdcardMounted",sdcardMounted));
    obj.push_back(Pair("bedTempSet",bed.tempSet));
    obj.push_back(Pair("bedTempRead",bed.tempRead));
    obj.push_back(Pair("speedMultiply",speedMultiply));
    obj.push_back(Pair("flowMultiply",flowMultiply));
    obj.push_back(Pair("numExtruder",printer->extruderCount));
    obj.push_back(Pair("firmware",firmware));
    obj.push_back(Pair("firmwareURL",firmwareURL));
    Array ea;
    for(int i=0;i<printer->extruderCount;i++) {
        Object e;
        e.push_back(Pair("tempSet",extruder[i].tempSet));
        e.push_back(Pair("tempRead",extruder[i].tempRead));
        e.push_back(Pair("output",extruder[i].output));
        ea.push_back(e);
    }
    obj.push_back(Pair("extruder",ea));
}
