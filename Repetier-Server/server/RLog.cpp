//
//  RLog.cpp
//  Repetier-Server
//
//  Created by Roland Littwin on 03.01.13.
//
//

#ifdef __unix
#include <syslog.h>
#endif
#include "RLog.h"
#include "global_config.h"

using namespace std;

RLog rlog;

RLog::RLog() {
#ifdef __unix
   openlog ("Repetier-Server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
#endif
}
RLog::~RLog() {
#ifdef __linux
   closelog ();
#endif
}
void RLog::log(const std::string &line,bool err) {
    if(gconfig->daemon == false) {
        cout << line << endl;
        return;
    }
#ifdef __unix
    if(err)
        syslog (LOG_ERR, "%s",line.c_str());
    else
        syslog (LOG_INFO,"%s", line.c_str());
#endif
}
void RLog::log(const std::string &line,int val,bool err) {
    string res;
    size_t p = line.find('@');
    if(p==string::npos) p = line.length();
    res = line.substr(0,p)+intToString(val)+line.substr(min(p+1,line.length()));
    log(res,err);
}
void RLog::log(const std::string &line,double val,bool err) {
    string res;
    size_t p = line.find('@');
    char buf[40];
    sprintf(buf,"%f",val);
    if(p==string::npos) p = line.length();
    res = line.substr(0,p)+static_cast<string>(buf)+line.substr(min(p+1,line.length()));
    log(res,err);
}
void RLog::log(const std::string &line,const std::string& val,bool err) {
    string res;
    size_t p = line.find('@');
    if(p==string::npos) p = line.length();
    res = line.substr(0,p)+val+line.substr(min(p+1,line.length()));
    log(res,err);
}