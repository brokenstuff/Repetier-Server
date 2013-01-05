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


#include <iostream>
#include <fstream>
#include "mongoose.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "printer.h"
#include "global_config.h"
#include "WebserverAPI.h"
#if defined(__APPLE__) || defined(__linux)
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#endif

namespace po = boost::program_options;
using namespace std;

struct mg_context *ctx;

static void *callback(enum mg_event event,
                      struct mg_connection *conn) {
    const struct mg_request_info *ri = mg_get_request_info(conn);
    
    if (event == MG_NEW_REQUEST) {
        if(strncmp(ri->uri,"/printer/",9)!=0) return repetier::HandlePagerequest(conn);
        repetier::HandleWebrequest(conn);
        // Mark as processed
        return (void*)"";
    } else {
        return NULL; //repetier::HandlePagerequest(conn);;
    }
}
#if defined(__APPLE__) || defined(__linux)

void Signal_Handler(int sig) /* signal handler function */
{
    switch(sig){
        case SIGHUP:
            break;
        case SIGTERM:
            mg_stop(ctx);
            gconfig->stopPrinterThreads();
            exit(0); // Terminate server
            break;		
    }	
}
#endif

int main(int argc, const char * argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("config,c", po::value<string>(), "Configuration file")
    ("daemon","Start as daemon")
    ("pidfile,p",po::value<string>(),"PID file")
    ("userid,u",po::value<string>(),"User id for daemon")
    ;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    } catch(std::exception &ex) {
        cerr << "error: Error parsing command line: " << ex.what() << endl;
    }
    po::notify(vm);
    
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    string confFile;
    if(vm.count("config")) {
        confFile = vm["config"].as<string>();
    }
    if(confFile.length() == 0) confFile = string("/etc/repetier-server.conf");
    boost::filesystem::path cf(confFile);
    if(!boost::filesystem::exists(cf)) {
        cerr << "Configuration file not found at " << confFile << endl;
        cerr << "Please use config option with correct path" << endl;
        cerr << desc << endl;
        return 2;
    }
    gconfig = new GlobalConfig(confFile); // Read global configuration
    
    if(vm.count("daemon")) {
        gconfig->daemon = true;
#ifdef DEBUG
        cout << "Running as daemon" << endl;
#endif
#if defined(__APPLE__) || defined(__linux)
        if(vm.count("userid")) {
            string suid = vm["pidfile"].as<string>();
            int uid = 0;
            sscanf(suid.c_str(),"%i",&uid);
            setuid(uid);
        }
        pid_t pid, sid;
        
        //Fork the Parent Process
        pid = fork();
        
        if (pid < 0) { exit(EXIT_FAILURE); }
        
        //We got a good pid, Close the Parent Process
        if (pid > 0) { exit(EXIT_SUCCESS); }
        
        //Change File Mask
        umask(0);
        
        //Create a new Signature Id for our child
        sid = setsid();
        if (sid < 0) { exit(EXIT_FAILURE); }
        
        //Change Directory
        //If we cant find the directory we exit with failure.
        if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
        
        //Close Standard File Descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        signal(SIGHUP,Signal_Handler); /* hangup signal */
        signal(SIGTERM,Signal_Handler); /* software termination signal from kill */
        if(vm.count("pidfile")) {
            try {
                pid = getpid();
                ofstream out(vm["pidfile"].as<string>().c_str());
                out << pid;
            } catch(std::exception &ex) {

            }
        }
#endif
    }
    
    gconfig->readPrinterConfigs();
    gconfig->startPrinterThreads();
    const char *options[] = {"document_root", gconfig->getWebsiteRoot().c_str(),"listening_ports", gconfig->getPorts().c_str(), NULL};
    
    ctx = mg_start(&callback, NULL, options);
    //getchar();  // Wait until user hits "enter"
    if(gconfig->daemon) {
        while(1) {
            sleep(1);
        }
    }
    while(true) {
        if(getchar()=='x') break;
    }
    mg_stop(ctx);
    cout << "Closing server" << endl;
    gconfig->stopPrinterThreads();
    return 0;
}

