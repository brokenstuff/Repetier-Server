/*
 Copyright 2012 Roland Littwin (repetier) repetierdev@gmail.com
 
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


#ifndef __Repetier_Server__GCode__
#define __Repetier_Server__GCode__

#include <iostream>

class Printer;

class GCodeDataPacket {
public:
    GCodeDataPacket(int _length,uint8_t *_data);
    ~GCodeDataPacket();
    int length;
    uint8_t *data;
};
class GCode {
    void ActivateV2OrForceAscii(Printer *printer);
    void parse(Printer *printer);
    void addCode(Printer *printer,char c,const std::string & val);
public:
    uint16_t fields,fields2;
    int32_t n;
    uint8_t t;
    uint16_t g, m;
    float x, y, z, e, f,ii,j,r;
    int32_t s;
    int32_t p;
    std::string text;
    std::string orig;
    bool comment;
    bool hostCommand;
    bool forceASCII;
    GCode(Printer &printer, std::string const &cmd);
    ~GCode();

    inline bool hasM()  {return (fields & 2)!=0;}
    inline bool hasN() {return (fields & 1)!=0;}
    inline bool hasG() {return (fields & 4)!=0;}
    inline bool hasT() {return (fields & 512)!=0;}
    inline bool hasX() {return (fields & 8)!=0;}
    inline bool hasY() {return (fields & 16)!=0;}
    inline bool hasZ() {return (fields & 32)!=0;}
    inline bool hasE() {return (fields & 64)!=0;}
    inline bool hasF() {return (fields & 256)!=0;}
    inline bool hasS() {return (fields & 1024)!=0;}
    inline bool hasP() {return (fields & 2048)!=0;}
    inline bool hasI() {return (fields2 & 1)!=0;}
    inline bool hasJ() {return (fields2 & 2)!=0;}
    inline bool hasR() {return (fields2 & 4)!=0;}
    inline bool hasText() {return (fields & 32768)!=0;}
    inline bool hasComment() {return comment;}
    inline bool isV2() {return (fields & 4096)!=0;}

    inline const std::string& getText() {return text;}
    inline uint16_t getG() {return g;}
    inline uint16_t getM() {return m;}
    inline uint8_t getT() {return t;}
    inline int32_t getN() {return n;}
    inline int32_t getS() {return s;}
    inline int32_t getP() {return p;}
    inline float getX() {return x;}
    inline float getY() {return y;}
    inline float getZ() {return z;}
    inline float getE() {return e;}
    inline float getF() {return f;}
    inline const std::string& getOriginal() {return orig;}
    void setN(int32_t line);
    GCodeDataPacket *getAscii(bool inclLine,bool inclChecksum);
    GCodeDataPacket *getBinary();
    std::string hostCommandPart();
    std::string hostParameter();
};
#endif /* defined(__Repetier_Server__GCode__) */
