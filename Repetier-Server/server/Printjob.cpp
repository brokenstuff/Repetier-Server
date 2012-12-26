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


#include "Printjob.h"
#include <boost/filesystem.hpp>
#include <vector>

using namespace std;
using namespace boost;
using namespace boost::filesystem;

typedef vector<path> pvec;             // store paths
typedef list<shared_ptr<Printjob>> pjlist;

PrintjobManager::PrintjobManager(string dir) {
    char lc = dir[dir.length()-1];
    if(lc=='/' || lc=='\\')
       dir = dir.substr(0,dir.length()-1);
    directory = dir;
    lastid = 0;
    path p(directory);
    try {
        if(!exists(p)) { // First call - create directory
            if(!create_directories(p)) {
                cerr << "error: Unable to create job directory " << dir << "." << endl;
                exit(-1);
            }
        }
        // Test if job directory is directory
        if(!is_directory(p)) {
            cerr << "error: " << dir << " is not a directory!" << endl;
            exit(-1);
        }
        // remove unfinished jobs
        cleanupUnfinsihed();
        // Read printjobs contained
        pvec v;
        copy(directory_iterator(p), directory_iterator(), back_inserter(v));
        sort(v.begin(), v.end());
        for (pvec::const_iterator it (v.begin()); it != v.end(); ++it)
        {
            shared_ptr<Printjob> pj(new Printjob((*it).native()));
            if(!pj->isNotExistent()) {
                files.push_back(pj);
            }
            // Extract id for last id;
            string sid = it->filename().native();
            size_t upos = sid.find('_');
            if(upos!=string::npos) {
                sid = sid.substr(0,upos);
                lastid = (size_t)atol(sid.c_str());
            }
        }
    } catch(const filesystem_error& ex)
    {
        cerr << "error: Unable to create or access job directory " << dir << "." << endl;
        exit(-1);
    }
}

void PrintjobManager::cleanupUnfinsihed() {
    path p(directory);
    pvec v;
    copy(directory_iterator(p), directory_iterator(), back_inserter(v));
    for (pvec::const_iterator it (v.begin()); it != v.end(); ++it)
    {
        if(it->extension()==".u") {
            remove(*it);
        }
    }
    
}

std::string PrintjobManager::encodeName(int id,std::string name,std::string postfix,bool withDir) {
    string res;
    char buf[16];
    sprintf(buf,"%08d_",id);
    if(withDir)
        res = directory+"/";
    res+=string(buf)+name+"."+postfix;
    return res;
}

std::string PrintjobManager::decodeNamePart(std::string file) {
    path p(file);
    string name = p.filename().stem().native();
    size_t upos = name.find('_');
    if(upos!=string::npos)
        name = name.substr(upos+1);
    return name;
}
int PrintjobManager::decodeIdPart(std::string file) {
    path p(file);
    string name = p.filename().stem().native();
    size_t upos = name.find('_');
    if(upos!=string::npos)
        name = name.substr(0,upos);
    else return -1;
    return atoi(name.c_str());
}
void PrintjobManager::fillSJONObject(std::string name,json_spirit::Object &o) {
    using namespace json_spirit;
    Array a;
    list<shared_ptr<Printjob>>::iterator it = files.begin(),ie = files.end();
    for(;it!=ie;it++) {
        Object j;
        Printjob *job = (*it).get();
        j.push_back(Pair("id",job->getId()));
        j.push_back(Pair("name",job->getName()));
        j.push_back(Pair("length",(int)job->getLength()));
        
        a.push_back(j);
    }
    o.push_back(Pair(name,a));
}
boost::shared_ptr<Printjob> PrintjobManager::findById(int id) {
    mutex::scoped_lock l(filesMutex);
    pjlist::iterator it = files.begin(),ie=files.end();
    for(;it!=ie;it++) {
        if((*it)->getId()==id)
            return *it;
    }
    return shared_ptr<Printjob>();
}
// ============= Printjob =============================

Printjob::Printjob(string _file) {
    file = _file;
    path p(file);
    pos = 0;
    state = stored;
    length = 0;
    id = PrintjobManager::decodeIdPart(file);
    try {
        if(exists(p) && is_regular_file(p))
            length = file_size(file);
        else state = doesNotExist;
    } catch(const filesystem_error& ex)
    {
        state = doesNotExist;
    }

}

std::string Printjob::getName() {
    return PrintjobManager::decodeNamePart(file);
}
