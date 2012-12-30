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


#ifndef __Repetier_Server__WebserverAPI__
#define __Repetier_Server__WebserverAPI__

#include <iostream>
namespace repetier {
    extern void HandleWebrequest(struct mg_connection *conn);
    extern void* HandlePagerequest(struct mg_connection *conn);
    /** Load the given file and translate contents. Store the translated file
     in a string. Function is thread safe.
     @param filename File to load and translate.
     @param lang Language code for translation.
     @param result File content as translated string.
     */
    extern void TranslateFile(const std::string &filename,const std::string &lang,std::string& result);
    extern bool MG_getVar(const mg_request_info *info,const char *name, std::string &output);
}

#endif /* defined(__Repetier_Server__WebserverAPI__) */
