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


#ifndef __Repetier_Server__Printjob__
#define __Repetier_Server__Printjob__

#include <iostream>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "json_spirit_value.h"

class Printjob {
public:
    enum PrintjobState {startUpload,stored,running,finsihed,doesNotExist};
    Printjob(std::string _file);
    bool isNotExistent() {return state==doesNotExist;}
    std::string getName();
    int getId() {return id;}
    size_t getLength() {return length;}
private:
    int id;
    std::string file;
    size_t length; ///< Length of the print file
    size_t pos; ///< Send until this position
    PrintjobState state;

};
/** The PrintjobManager manages a directory full of possible print jobs.
 The files follow a naming convention to force the order of files added.
 Each file has a number_<Name>.<state>
 
 state is u for the time until it is uploaded completely and gets renamed to g
 after upload is complete. At the start all files with .u get deleted as they
 never finished.
 */
class PrintjobManager {
    std::string directory;
    std::list<boost::shared_ptr<Printjob>> files;
    size_t lastid;
    boost::mutex filesMutex;
public:
    PrintjobManager(std::string dir);
    void cleanupUnfinsihed();
    std::string encodeName(int id,std::string name,std::string postfix,bool withDir);
    static std::string decodeNamePart(std::string file);
    static int decodeIdPart(std::string file);
    void fillSJONObject(std::string name,json_spirit::Object &o);
    boost::shared_ptr<Printjob> findById(int id);
};
#endif /* defined(__Repetier_Server__Printjob__) */
