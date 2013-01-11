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

#include "Printjob.h"
#include <boost/filesystem.hpp>
#include <vector>
#include "printer.h"
#include "global_config.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warning in VS2005
#endif

typedef vector<path> pvec;             // store paths
typedef list<shared_ptr<Printjob> > pjlist;

PrintjobManager::PrintjobManager(string dir,Printer *_prt,bool _scripts) {
    scripts = _scripts;
    printer = _prt;
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
        if(scripts) {
            bool hasStart = false,hasEnd = false,hasPause=false,hasKill=false;
            bool hasScript1=false,hasScript2=false,hasScript3=false,hasScript4=false,hasScript5;
            for (pvec::const_iterator it (v.begin()); it != v.end(); ++it)
            {
                PrintjobPtr pj(new Printjob((*it).string(),false));
                files.push_back(pj);
                string name = it->filename().string();
                if(name=="Start.g") hasStart = true;
                if(name=="End.g") hasEnd = true;
                if(name=="Pause.g") hasPause = true;
                if(name=="Kill.g") hasKill = true;
                if(name=="Script 1.g") hasScript1 = true;
                if(name=="Script 2.g") hasScript2 = true;
                if(name=="Script 3.g") hasScript3 = true;
                if(name=="Script 4.g") hasScript4 = true;
                if(name=="Script 5.g") hasScript5 = true;
            }
            if(!hasStart) {files.push_back(PrintjobPtr(new Printjob(directory+"/Start.g",true,true)));}
            if(!hasEnd) {files.push_back(PrintjobPtr(new Printjob(directory+"/End.g",true,true)));}
            if(!hasPause) {files.push_back(PrintjobPtr(new Printjob(directory+"/Pause.g",true,true)));}
            if(!hasKill) {files.push_back(PrintjobPtr(new Printjob(directory+"/Kill.g",true,true)));}
            if(!hasScript1) {files.push_back(PrintjobPtr(new Printjob(directory+"/Script 1.g",true,true)));}
            if(!hasScript2) {files.push_back(PrintjobPtr(new Printjob(directory+"/Script 2.g",true,true)));}
            if(!hasScript3) {files.push_back(PrintjobPtr(new Printjob(directory+"/Script 3.g",true,true)));}
            if(!hasScript4) {files.push_back(PrintjobPtr(new Printjob(directory+"/Script 4.g",true,true)));}
            if(!hasScript5) {files.push_back(PrintjobPtr(new Printjob(directory+"/Script 5.g",true,true)));}
        } else {
            for (pvec::const_iterator it (v.begin()); it != v.end(); ++it)
            {
                PrintjobPtr pj(new Printjob((*it).string(),false));
                if(!pj->isNotExistent()) {
                    files.push_back(pj);
                }
                // Extract id for last id;
                string sid = it->filename().string();
                size_t upos = sid.find('_');
                if(upos!=string::npos) {
                    sid = sid.substr(0,upos);
                    lastid = atoi(sid.c_str());
                }
            }
        }
    } catch(const filesystem_error& ex)
    {
		cerr << "error: Unable to create or access job directory " << dir << ":" << ex.what() << endl;
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
    string name = p.filename().stem().string();
    size_t upos = name.find('_');
    if(upos!=string::npos)
        name = name.substr(upos+1);
    return name;
}
int PrintjobManager::decodeIdPart(std::string file) {
    path p(file);
    string name = p.filename().stem().string();
    size_t upos = name.find('_');
    if(upos!=string::npos)
        name = name.substr(0,upos);
    else return -1;
    return atoi(name.c_str());
}
void PrintjobManager::fillSJONObject(std::string name,json_spirit::Object &o) {
    mutex::scoped_lock l(filesMutex);
    using namespace json_spirit;
    Array a;
    list<shared_ptr<Printjob> >::iterator it = files.begin(),ie = files.end();
    for(;it!=ie;it++) {
        Object j;
        Printjob *job = (*it).get();
        j.push_back(Pair("id",job->getId()));
        j.push_back(Pair("name",job->getName()));
        j.push_back(Pair("length",(int)job->getLength()));
        switch(job->getState()) {
            case Printjob::startUpload:
                j.push_back(Pair("state","uploading"));
                break;
            case Printjob::stored:
                j.push_back(Pair("state","stored"));
                break;
            case Printjob::running:
                j.push_back(Pair("state","running"));
                j.push_back(Pair("done",job->percentDone()));
                break;
            case Printjob::finished:
                j.push_back(Pair("state","finsihed"));
                break;
            case Printjob::doesNotExist:
                j.push_back(Pair("state","error"));
                break;
        }
        a.push_back(j);
    }
    o.push_back(Pair(name,a));
}
void PrintjobManager::getJobStatus(json_spirit::Object &obj) {
    mutex::scoped_lock l(filesMutex);
    using namespace json_spirit;
    Printjob *job = runningJob.get();
    if(job==NULL) {
        obj.push_back(Pair("job","none"));
    } else {
        obj.push_back(Pair("job",job->getName()));
        obj.push_back(Pair("done",job->percentDone()));
    }
}
PrintjobPtr PrintjobManager::findByIdInternal(int id) {
    pjlist::iterator it = files.begin(),ie=files.end();
    for(;it!=ie;it++) {
        if((*it)->getId()==id)
            return *it;
    }
    return shared_ptr<Printjob>();
}
PrintjobPtr PrintjobManager::findByName(string name) {
    mutex::scoped_lock l(filesMutex);
    pjlist::iterator it = files.begin(),ie=files.end();
    for(;it!=ie;it++) {
        if((*it)->getName()==name)
            return *it;
    }
    return shared_ptr<Printjob>();
}
PrintjobPtr PrintjobManager::findById(int id) {
    mutex::scoped_lock l(filesMutex);
    return findByIdInternal(id);
}
PrintjobPtr PrintjobManager::createNewPrintjob(std::string name) {
    mutex::scoped_lock l(filesMutex);
    lastid++;
    PrintjobPtr job(new Printjob(encodeName(lastid, name, "u", true),true));
    files.push_back(job);
    return job;
}
void PrintjobManager::finishPrintjobCreation(PrintjobPtr job,string namerep,size_t sz)
{
    mutex::scoped_lock l(filesMutex);
    if(job->getName().length()>0)
        namerep = job->getName();
    if(namerep.length()==0) {
        char buf[50];
        sprintf(buf,"Job %d",job->getId());
        namerep = static_cast<string>(buf);
    }
    string newname = encodeName(job->getId(),namerep,"g", true);
    rename(job->getFilename(), newname);
    job->setFilename(newname);
    job->setLength(sz);
    job->setStored();
}
void PrintjobManager::RemovePrintjob(PrintjobPtr job) {
    mutex::scoped_lock l(filesMutex);
    path p(job->getFilename());
    if(exists(p) && is_regular_file(p))
        remove(p);
    files.remove(job);
}
void PrintjobManager::startJob(int id) {
    mutex::scoped_lock l(filesMutex);
    if(runningJob.get()) return; // Can't start if old job is running
    runningJob = findByIdInternal(id);
    if(!runningJob.get()) return; // unknown job
    runningJob->setRunning();
    runningJob->start();
    jobin.open(runningJob->getFilename().c_str(),ifstream::in);
}
void PrintjobManager::killJob(int id) {
    mutex::scoped_lock l(filesMutex);
    if(!runningJob.get()) return; // Can't start if old job is running
    if(jobin.is_open() && jobin.eof()) {
        jobin.close();
    }
    files.remove(runningJob);
    remove(path(runningJob->getFilename())); // Delete file from disk
    runningJob.reset();
    printer->getScriptManager()->pushCompleteJob("End");
}
void PrintjobManager::undoCurrentJob() {
    mutex::scoped_lock l(filesMutex);
    if(!runningJob.get()) return; // no running job
    if(jobin.is_open() && jobin.eof()) {
        jobin.close();
    }
    runningJob->setStored();
    files.remove(runningJob);
    runningJob.reset();
}
void PrintjobManager::manageJobs() {
    mutex::scoped_lock l(filesMutex);
    if(!runningJob.get()) return; // unknown job
    if(jobin.good()) {
        string line;
        size_t n = 100-printer->jobCommandsStored();
        if(n>10) n = 10;
        char buf[200];
        while(n && !jobin.eof()) {
            jobin.getline(buf, 200); // Strips \n
            size_t l = strlen(buf);
            if(buf[l]=='\r')
                buf[l] = 0;
            printer->injectJobCommand(static_cast<string>(buf));
            runningJob->incrementLinesSend();
            n--;
        }
        runningJob->setPos(jobin.tellg());
    }
    if(jobin.is_open() && !jobin.good()) {
        jobin.close();
        files.remove(runningJob);
        runningJob->stop(printer);
        remove(path(runningJob->getFilename())); // Delete file from disk
        runningJob.reset();
        l.unlock();
        printer->scriptManager->pushCompleteJobNoBlock("End");
    }
}
void PrintjobManager::pushCompleteJob(std::string name,bool beginning) {
    PrintjobPtr pj = findByName(name);
    if(!pj.get()) return;
    mutex::scoped_lock l(filesMutex);
    ifstream in;
    mutex::scoped_lock l2(printer->sendMutex);
    std::deque<std::string> &list = ((*pj).getName()=="Pause" ? printer->manualCommands : printer->jobCommands);
    try {
        in.open(pj->getFilename().c_str(),ifstream::in);
        char buf[200];
        size_t line = 0;
        
        while(!in.eof()) {
            in.getline(buf, 200); // Strips \n
            size_t l = strlen(buf);
            if(buf[l]=='\r')
                buf[l] = 0;
            string cmd(buf);
            if(!printer->shouldInjectCommand(cmd)) continue;
            line++;
            if(beginning) {
                list.push_front(cmd);
            } else {
                list.push_back(cmd);
            }
        }
        if(beginning) { // Reverse lines at beginning
            reverse(printer->jobCommands.begin(),printer->jobCommands.begin()+line);
        }
        in.close();
    } catch(std::exception) {}
}
void PrintjobManager::pushCompleteJobNoBlock(std::string name,bool beginning) {
    PrintjobPtr pj = findByName(name);
    if(!pj.get()) return;
    mutex::scoped_lock l(filesMutex);
    ifstream in;
    std::deque<std::string> &list = ((*pj).getName()=="Pause" ? printer->manualCommands : printer->jobCommands);
    try {
        in.open(pj->getFilename().c_str(),ifstream::in);
        char buf[200];
        size_t line = 0;
        
        while(!in.eof()) {
            in.getline(buf, 200); // Strips \n
            size_t l = strlen(buf);
            if(buf[l]=='\r')
                buf[l] = 0;
            string cmd(buf);
            if(!printer->shouldInjectCommand(cmd)) continue;
            line++;
            if(beginning) {
                list.push_front(cmd);
            } else {
                list.push_back(cmd);
            }
        }
        if(beginning) { // Reverse lines at beginning
            reverse(printer->jobCommands.begin(),printer->jobCommands.begin()+line);
        }
        in.close();
    } catch(std::exception) {}
}
// ============= Printjob =============================

Printjob::Printjob(string _file,bool newjob,bool _script) {
    file = _file;
    script = _script;
    path p(file);
    pos = 0;
    state = stored;
    length = 0;
    if(script) id=0;
    else id = PrintjobManager::decodeIdPart(file);
    if(script && newjob) {
        fstream f;
        f.open( file.c_str(), ios::out );
        f << flush;
        f.close();
        return;
    }
    else if(newjob) {state = startUpload; return;}
    try {
        if(exists(p) && is_regular_file(p))
            length = (size_t)file_size(file);
        else state = doesNotExist;
    } catch(const filesystem_error& )
    {
        state = doesNotExist;
    }

}

std::string Printjob::getName() {
    return PrintjobManager::decodeNamePart(file);
}

void Printjob::start() {
    linesSend = 0;
    time = boost::posix_time::microsec_clock::local_time();
}
void Printjob::stop(Printer *p) {
    posix_time::ptime now  = boost::posix_time::microsec_clock::local_time();
    posix_time::time_duration td(now-time);
    char b[100];
    sprintf(b,"%d:%02d:%02d",td.hours(),td.minutes(),td.seconds());
    string msg = "Print of "+getName()+" on printer "+p->name+ " finished. Send "+intToString(linesSend)+" lines. Printing time: "+b;
    string url = "/printer/msg/"+p->slugName+"?a=jobfinsihed";
    gconfig->createMessage(msg, url);
}
