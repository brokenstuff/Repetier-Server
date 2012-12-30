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

#include "mongoose.h"
#include "WebserverAPI.h"
#include "global_config.h"
#include "printer.h"
#include "json_spirit.h"
#include <map>
#include "moFileReader.h"
#include <boost/bind.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "PrinterState.h"
#include "Printjob.h"
#if defined(_WIN32)
#include <io.h>
#endif

using namespace std;
using namespace json_spirit;
using namespace boost;

#if defined(_WIN32) && !defined(__SYMBIAN32__) // Windows specific
#pragma warning( disable : 4996 )
// Visual Studio 6 does not know __func__ or __FUNCTION__
// The rest of MS compilers use __FUNCTION__, not C99 __func__
// Also use _strtoui64 on modern M$ compilers
#if defined(_MSC_VER) && _MSC_VER < 1300
#define STRX(x) #x
#define STR(x) STRX(x)
#define __func__ "line " STR(__LINE__)
#define strtoull(x, y, z) strtoul(x, y, z)
#define strtoll(x, y, z) strtol(x, y, z)
#else
#define __func__  __FUNCTION__
#define strtoull(x, y, z) _strtoui64(x, y, z)
#define strtoll(x, y, z) _strtoi64(x, y, z)
#endif // _MSC_VER

#define O_EXLOCK 0
#define O_CLOEXEC 0
#define O_CREAT _O_CREAT
#define O_APPEND _O_APPEND
#define O_TRUNC _O_TRUNC
#define O_RDWR _O_RDWR
#define snprintf _snprintf
#define strnstr _strnstr
#define popen(x, y) _popen(x, y)
#define pclose(x) _pclose(x)
#define myclose(x) _close(x)
#define myopen(x,y,z) _open(x,y,z)
#define fseeko(x, y, z) _lseeki64(_fileno(x), (y), (z))
#define fdopen(x, y) _fdopen((x), (y))
#define mywrite(x, y) _write((x), (y))
//#define read(x, y, z) _read((x), (y), (unsigned) z)
#else
#define myclose(x) close(x)
#define myopen(x,y,z) open(x,y,z)
#define mywrite(x, y) write((x), (y))
#endif
#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif

namespace repetier {
    static const char *HTTP_500 = "HTTP/1.0 500 Server Error\r\n\r\n";
    
	char *mystrnstr(const char *s,const char *needle,int len) {
		int ln = (int)strlen(needle);
		if(!ln) return (char*)s;
		char *s2 = (char *)s;
		bool ok = false;
		for(int o=0;o<len-ln;o++) {
			s2 = (char*)&s[o];
			bool ok = true;
			for(int j=0;j<ln && ok;j++) {
				ok = s2[j]==needle[j];
			}
			if(ok) break;
		}
		if(!ok) return NULL;
		return s2;
	}

    // Modified verion from mongoose examples
    bool handleFileUpload(struct mg_connection *conn,const string& filename,string& name,long &size,bool append) {
        name.clear();
        const char *cl_header;
        char post_data[16 * 1024], path[999], file_name[1024], mime_type[100],boundary[100],
        buf[BUFSIZ*2], *eop, *s, *p;
        FILE *fp;
        long long int cl, written;
        int fd, n, post_data_len;
        
        // Figure out total content length. Return if it is not present or invalid.
        cl_header = mg_get_header(conn, "Content-Length");
        if (cl_header == NULL || (cl = strtoll(cl_header, NULL, 10)) <= 0) {
            mg_printf(conn, "%s%s", HTTP_500, "Invalid Conent-Length");
            return false;
        }
        
        // Read the initial chunk into memory. This should be multipart POST data.
        // Parse headers, where we should find file name and content-type.
        post_data_len = mg_read(conn, post_data, sizeof(post_data));
        file_name[0] = mime_type[0] = '\0';
        for (s = p = post_data; p < &post_data[post_data_len]; p++) {
            if (p[0] == '\r' && p[1] == '\n') {
                if (s == p && *file_name) {
                    p += 2;
                    break;  // End of headers
                }
                p[0] = p[1] = '\0';
                sscanf(s, "Content-Type: %99s", mime_type);
                // TODO(lsm): don't expect filename to be the 3rd field,
                // parse the header properly instead.
                sscanf(s, "Content-Disposition: %*s %*s filename=\"%1023[^\"]",
                       file_name);
                s = p + 2;
            }
        }
        if(strlen(post_data)>100) {
            mg_printf(conn, "%s%s", HTTP_500, "Boundary too long");
            return false;
        }
        boundary[0] = '\r';
        boundary[1] = '\n';
        strcpy(&boundary[2],post_data);
        // Finished parsing headers. Now "p" points to the first byte of data.
        // Calculate file size
        cl -= p - post_data;      // Subtract headers size
        cl -= strlen(post_data);  // Subtract the boundary marker at the end
        cl -= 6;                  // Subtract "\r\n" before and after boundary
        
        // Construct destination file name. Write to /tmp, do not allow
        // paths that contain slashes.
        if ((s = strrchr(file_name, '/')) == NULL) {
            s = file_name;
        }
        snprintf(path, sizeof(path), "/tmp/%s", s);
        
        if (file_name[0] == '\0') {
            mg_printf(conn, "%s%s", HTTP_500, "Can't get file name");
        } else if (cl <= 0) {
            mg_printf(conn, "%s%s", HTTP_500, "Empty file");
        } else if ((fd = myopen(filename.c_str(), O_CREAT | (append ? O_APPEND : O_TRUNC) |
                              O_RDWR /*| O_WRONLY | O_EXLOCK | O_CLOEXEC*/,0666)) < 0) {
            // We're opening the file with exclusive lock held. This guarantee us that
            // there is no other thread can save into the same file simultaneously.
            mg_printf(conn, "%s%s", HTTP_500, "Cannot open file");
        } else if ((fp = fdopen(fd,(append ? "a+" : "w"))) == NULL) {
            mg_printf(conn, "%s%s", HTTP_500, "Cannot reopen file stream");
            myclose(fd);
        } else {
            bool finished = false;
            name = file_name;
            int boundlen = (int)strlen(boundary);
            // Success. Write data into the file.
            eop = post_data + post_data_len;
            n = p + cl > eop ? (int) (eop - p) : (int) cl;
            char *p2 = mystrnstr(p,boundary,n);
            int startnew = 0;
            if(p2!=NULL) { // End boundary detected
                finished = true;
                n = (int)(p2-p);
            } else if(n>boundlen) {
                n-=boundlen;
                startnew = boundlen;
            }
            (void) fwrite(p, 1, n, fp);
            written = n;
            if(!finished)
                memcpy(buf,&p[n],boundlen);
            while (!finished && written < cl &&
                   (n = mg_read(conn, &buf[startnew],(size_t)( cl - written > (long long) sizeof(buf)-startnew ?
                                sizeof(buf)-startnew : cl - written)) > 0)) {
                n+=startnew;
                p2 = mystrnstr(buf,boundary,n);
                startnew = 0;
                if(p2!=NULL) { // End boundary detected
                    finished = true;
                    n = (int)(p2-buf);
                } else if(n>boundlen) {
                    n-=boundlen;
                    startnew = boundlen;
                }

                (void) fwrite(buf, 1, n, fp);
                written += n;
                if(!finished)
                    memcpy(buf,&buf[n],boundlen);
            }
            (void) fclose(fp);
            size = (long)written;
            return true;
            //mg_printf(conn, "HTTP/1.0 200 OK\r\n\r\n"
            //        "Saved to [%s], written %llu bytes", path, cl);
        }
        return false;
    }
    void listPrinter(Object &ret) {
        Array parr;
        std::vector<Printer*> *list = &gconfig->getPrinterList();
        for(vector<Printer*>::iterator i=list->begin();i!=list->end();++i) {
            Object pinfo;
            Printer *p = *i;
            pinfo.push_back(Pair("name",p->name));
            pinfo.push_back(Pair("slug",p->slugName));
            pinfo.push_back(Pair("online",(p->getOnlineStatus())));
            pinfo.push_back(Pair("job",p->getJobStatus()));
            pinfo.push_back(Pair("active",p->getActive()));
            parr.push_back(pinfo);
        }
        ret.push_back(Pair("data",parr));
        Array msg;
        gconfig->fillJSONMessages(msg);
        ret.push_back(Pair("messages",msg));
    }
    void HandleWebrequest(struct mg_connection *conn) {
        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
                  "Cache-Control:public, max-age=0\r\n"
                  "Server: Repetier-Server\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n\r\n");
        const struct mg_request_info *ri = mg_get_request_info(conn);
        // First check the path for its parts
        int start(9); // /printer/
        int end(9);
        string error;
        Printer *printer = NULL;
        const char* uri = ri->uri;
        while(uri[end] && uri[end]!='/') end++;
        string cmdgroup(&uri[start],end-start);
        if(uri[end]) { // Read printer string
            start = end = end+1;
            while(uri[end] && uri[end]!='/') end++;
            printer = gconfig->findPrinterSlug(string(&uri[start],end-start));        
        }
        Object ret;
        if(cmdgroup=="list") {
            listPrinter(ret);
        } else if(printer==NULL) {
            error = "Unknown printer";
        } else if(cmdgroup=="job") {
            string a;
            MG_getVar(ri,"a",a);
            if(a=="list") {
                printer->getJobManager()->fillSJONObject("data",ret);
                Array msg;
                gconfig->fillJSONMessages(msg);
                ret.push_back(Pair("messages",msg));
            } else if(a=="upload") {
#ifdef DEBUG
                cout << "Upload job" << endl;
#endif
                string name,jobname;
                long size;
                MG_getVar(ri,"name", jobname);
                PrintjobPtr job = printer->getJobManager()->createNewPrintjob(jobname);
                handleFileUpload(conn,job->getFilename(), name,size,false);
                printer->getJobManager()->finishPrintjobCreation(job,name,size);
                printer->getJobManager()->fillSJONObject("data",ret);
#ifdef DEBUG
                cout << "Name:" << name << " Size:" << size << endl;
#endif
            } else if(a=="remove") {
                string sid;
                if(MG_getVar(ri,"id",sid)) {
                    int id = atoi(sid.c_str());
                    PrintjobPtr job = printer->getJobManager()->findById(id);
                    if(job.get())
                        printer->getJobManager()->RemovePrintjob(job);
                }
                printer->getJobManager()->fillSJONObject("data",ret);
            } else if(a=="start") {
                string sid;
                if(MG_getVar(ri,"id",sid)) {
                    int id = atoi(sid.c_str());
                    PrintjobPtr job = printer->getJobManager()->findById(id);
                    if(job.get())
                        printer->getJobManager()->startJob(id);
                }
                printer->getJobManager()->fillSJONObject("data",ret);
            } else if(a=="kill") {
                string sid;
                if(MG_getVar(ri,"id",sid)) {
                    int id = atoi(sid.c_str());
                    PrintjobPtr job = printer->getJobManager()->findById(id);
                    if(job.get())
                        printer->getJobManager()->killJob(id);
                }
                printer->getJobManager()->fillSJONObject("data",ret);                
            }
        } else if(cmdgroup=="model") {
            string a;
            MG_getVar(ri,"a",a);
            if(a=="list") {
                printer->getModelManager()->fillSJONObject("data",ret);
            } else if(a=="upload") {
#ifdef DEBUG
                cout << "Upload model" << endl;
#endif
                string name,jobname;
                long size=0;
                MG_getVar(ri,"name", jobname);
                PrintjobPtr job = printer->getModelManager()->createNewPrintjob(jobname);
                handleFileUpload(conn,job->getFilename(), name,size,false);
                printer->getModelManager()->finishPrintjobCreation(job,name,size);
                printer->getModelManager()->fillSJONObject("data",ret);
#ifdef DEBUG
                cout << "Name:" << name << " Size:" << size << endl;
#endif
            } else if(a=="remove") {
                string sid;
                if(MG_getVar(ri,"id",sid)) {
                    int id = atoi(sid.c_str());
                    PrintjobPtr job = printer->getModelManager()->findById(id);
                    if(job.get())
                        printer->getModelManager()->RemovePrintjob(job);
                }	
                printer->getModelManager()->fillSJONObject("data",ret);
            } else if(a=="copy") {
                string sid;
                if(MG_getVar(ri,"id",sid)) {
                    int id = atoi(sid.c_str());
                    PrintjobPtr model = printer->getModelManager()->findById(id);
                    if(model.get()) {
                        PrintjobPtr job = printer->getJobManager()->createNewPrintjob(model->getName());
                        job->setLength(model->getLength());
                        try {
                            std::ifstream  src(model->getFilename().c_str());
                            std::ofstream  dst(job->getFilename().c_str());
                            dst << src.rdbuf();
                            printer->getJobManager()->finishPrintjobCreation(job, model->getName(), model->getLength());
                        } catch(const std::exception& ex)
                    {
                        cerr << "error: Unable to create job file " << job->getFilename() << ":" << ex.what() << endl;
                    }
                    }
                }
                printer->getJobManager()->fillSJONObject("data",ret);
            }
        } else if(cmdgroup=="msg") {
            string a;
            string sid;
            MG_getVar(ri,"a",a);
            MG_getVar(ri,"id",sid);
            int id = 0;
            if(sid.length()>0) id = atoi(sid.c_str());
            if(a=="unpause") {
                printer->stopPause();
            }
            if(id) {
                gconfig->removeMessage(id);
                Array msg;
                gconfig->fillJSONMessages(msg);
                ret.push_back(Pair("messages",msg));
            }
        } else if(cmdgroup == "pconfig") {
            string a;
            MG_getVar(ri,"a",a);
            if(a=="active") {
                string mode;
                if(MG_getVar(ri, "mode",mode)) {
                    printer->setActive(mode=="1");
                }
                listPrinter(ret);
            }
        } else if(printer->getOnlineStatus()==0) {
            error = "Printer offline";
            // ============ ONLINE COMMANDS FROM HERE ==============
        } else if(cmdgroup=="send") {
            string cmd;
            if(MG_getVar(ri,"cmd", cmd)) {
                printer->injectManualCommand(cmd);
            }
        } else if(cmdgroup=="response") { // Return log
            string sfilter,sstart;
            uint8_t filter=0;
            uint32_t start=0;
            if(MG_getVar(ri,"filter",sfilter))
                filter = atoi(sfilter.c_str());
            if(MG_getVar(ri,"start",sstart))
                start = (uint32_t)atol(sstart.c_str());
            boost::shared_ptr<list<boost::shared_ptr<PrinterResponse> > > rlist = printer->getResponsesSince(start,filter, start);
            Object lobj;
            lobj.push_back(Pair("lastid",(int)start));
            Array a;
            list<boost::shared_ptr<PrinterResponse> >::iterator it = rlist->begin(),end=rlist->end();
            for(;it!=end;++it) {
                PrinterResponse *resp = (*it).get();
                Object o;
                o.push_back(Pair("id",(int)resp->responseId));
                o.push_back(Pair("time",resp->getTimeString()));
                o.push_back(Pair("text",resp->message));
                o.push_back(Pair("type",resp->logtype));
                a.push_back(o);
            }
            lobj.push_back(Pair("lines",a));
            Object state;
            printer->state->fillJSONObject(state);
            lobj.push_back(Pair("state",state));
            ret.push_back(Pair("data",lobj));
        } else if(cmdgroup=="move") {
            string sx,sy,sz,se;
            double x=0,y=0,z=0,e=0;
            if(MG_getVar(ri,"x",sx)) x = atof(sx.c_str());
            if(MG_getVar(ri,"y",sx)) y = atof(sy.c_str());
            if(MG_getVar(ri,"z",sx)) z = atof(sz.c_str());
            if(MG_getVar(ri,"e",sx)) e = atof(se.c_str());
            printer->move(x, y, z, e);
        }
        ret.push_back(Pair("error",error));
    
        // Print result
		mg_printf(conn,"%s",write(ret,json_spirit::raw_utf8).c_str());
    }
    bool MG_getVar(const mg_request_info *info,const char *name, std::string &output)
    {
        output.clear();
        if(info->query_string==NULL) return false;
        const int MAX_VAR_LEN = 4096;
        char buffer[MAX_VAR_LEN];
        int len = mg_get_var(info->query_string, strlen(info->query_string), name, buffer, MAX_VAR_LEN - 1);
        if (len >= 0) {
            output.append(buffer, len);
            return true;
        }
        return false;
    }
    string JSONValueAsString(const Value &v) {
        switch(v.type()) {
            case str_type:
                return v.get_str();
            case int_type:
            {
                char b[40];
                sprintf(b,"%d",v.get_int());
                return string(b);
            }
            case real_type: {
                char b[40];
                sprintf(b,"%f",v.get_real());
                return string(b);
            }
            case bool_type:
                if(v.get_bool()) return string("true");
                return string("false");
            case array_type:
                return "array";
            case obj_type:
                return "object";
            case null_type:
                return "null";
        }
        return("Unsupported type");
    }
    Value* findVariable(list<Value> &vars,const string& name) {
        list<Value>::iterator istart = vars.begin(),iend = vars.end();
        bool found = false;
        for(;istart!=iend && !found;istart++) {
            Value &v = *istart;
            if(v.type()==obj_type) {
                Object &obj = v.get_obj();
                vector<Pair>::iterator oit = obj.begin(),oend = obj.end();
                for(;oit!=oend;oit++) {
                    Pair &p = *oit;
                    if(p.name_ == name) {
                        return &p.value_;
                    }
                }
            }
        }
        return NULL;
    }
    void FillTemplateRecursive(string& text,string& result,list<Value>& vars,size_t start,size_t end) {
        size_t pos(start),pos2,posclose;
        while(pos<end) {
            pos2 = text.find("{{",pos);
            if(pos2==string::npos || pos2+3>=end) { // Finished, no more vars etc
                result.append(text,pos,end-pos);
                return;
            }
            pos2+=2;
            posclose = text.find("}}",pos2);
            if(posclose==string::npos) { // Finished, no more vars etc
                result.append(text,pos,end-pos);
                return;
            }
            
            result.append(text,pos,pos2-pos-2);
            char tp = text[pos2];
            
            if(tp == '#') { // foreach loop
                string name = text.substr(pos2+1,posclose-pos2-1);
                string ename = "{{/"+name+"}}";
                size_t epos = text.find(ename,posclose);
                pos2+=name.length()+3;
                posclose = epos+ename.length()-2; // Continue after block
                Value *v = findVariable(vars,name);
                if(v->type()==array_type) {
                    Array &a = v->get_array();
                    vector<Value>::iterator it = a.begin(),iend = a.end();
                    for(;it!=iend;++it) {
                        vars.push_front(*it);
                        FillTemplateRecursive(text, result, vars, pos2, epos);
                        vars.pop_front();
                    }
                }
            } else if(tp=='!') { // Comment, simply ignore it
            } else { // Variable
                string name = text.substr(pos2,posclose-pos2);
                Value *v = findVariable(vars,name);
                if(v!=NULL)
                    result.append(JSONValueAsString(*v));
            }
            pos = posclose+2;
        }
    }
    void FillTemplate(string &text,string& result,Object& data) {
        result.clear();
        size_t start(0),end(text.length());
        result.reserve(end*2);
        list<Value> vars;
        vars.push_front(Value(data));
        FillTemplateRecursive(text,result,vars,start,end);
    }
    void* HandlePagerequest(struct mg_connection *conn) {
        const struct mg_request_info *ri = mg_get_request_info(conn);
        string uri(ri->uri);
        if(uri.length()<=1) uri="/index.php";
        if(uri.length()<5 || uri.substr(uri.length()-4,4)!=".php") return NULL;        
        
        // Step 1: Find translation file
        const char *alang = mg_get_header(conn, "Accept-Language");
        string lang = gconfig->getDefaultLanguage();
        if(alang!=NULL) {
            
        }
        string content;
        TranslateFile(gconfig->getWebsiteRoot()+uri,lang,content);
        // Step 2: Fill template parameter
        Object obj;
        string param;
        if(MG_getVar(ri,"pn", param)) {
            Printer *p = gconfig->findPrinterSlug(param);
            if(p) p->fillJSONObject(obj);
        }
        obj.push_back(Pair("version",string(REPETIER_SERVER_VERSION)));
        // Step 3: Run template
        string content2;
        FillTemplate(content, content2, obj);
        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
                  "Cache-Control:public, max-age=0\r\n"
                  "Server: Repetier-Server\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n\r\n");
        mg_write(conn, content2.c_str(), content2.length());
        return (void*)"";
    }
    void TranslateFile(const std::string &filename,const std::string &lang,std::string& result) {
        static map<string,boost::shared_ptr<moFileLib::moFileReader> > rmap;
        result.clear();
        
        // read mo file if not cached
        moFileLib::moFileReader *r = NULL;
        if((rmap[lang].get()) == NULL) {
            static boost::mutex mutex;
            boost::mutex::scoped_lock l(mutex);
            string mofile = gconfig->getLanguageDir()+lang+".mo";
            if(!boost::filesystem::exists(mofile))
                mofile = gconfig->getLanguageDir()+gconfig->getDefaultLanguage()+".mo";
            if(!boost::filesystem::exists(mofile)) return;
            r = new moFileLib::moFileReader();
            r->ReadFile(mofile.c_str());
            rmap[lang].reset(r);
        } else r = rmap[lang].get();
        // Read file contents
        string contents;
		std::ifstream in(filename.c_str(), ios::in | ios::binary);
		if (in.good())
        {
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
        } else {
            result.clear();
            return;
        }
        // Replace translations
        size_t start(0),pos(0),end(contents.length()),pos2,tstart,tend;
        result.reserve(end+end/10); // Reserve some extra space to prevent realloc
        while(pos<end) {
            pos = contents.find("<?php",start);
            if(pos==string::npos) { // End reached, copy rest of file
                result.append(contents,start,end-start);
                break;
            }
            pos2 = contents.find("?>",pos+5);
            if(pos2==string::npos) { // format error, copy rest of file
                result.append(contents,start,end-start);
                break;
            }
            result.append(contents,start,pos-start);
            start = pos2+2;
            tstart = contents.find("_(\"",pos);
            tend = contents.rfind("\")",pos2);
            if(tstart<tend && tend!=string::npos) {
                string key = contents.substr(tstart+3,tend-tstart-3);
                result.append(r->Lookup(key.c_str()));
            }
        }
    }
} // repetier namespace

