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

#include "global_config.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

GlobalConfig *gconfig;

GlobalConfig::GlobalConfig(string filename) {
	try {
		config.readFile(filename.c_str());
	} catch(libconfig::ParseException &pe) {
		cerr << "error: " << pe.getError() << " line:" << pe.getLine() << " file:" << pe.getFile() << endl;
		exit(-1);
	}
    bool ok = true;
    ok &= config.lookupValue("printer_config_directory",printerConfigDir);
    ok &= config.lookupValue("data_storage_directory",storageDir);
    ok &= config.lookupValue("website_directory", wwwDir);
    ok &= config.lookupValue("languages_directory", languageDir);
    ok &= config.lookupValue("default_language", defaultLanguage);
    ok &= config.lookupValue("ports",ports);
    backlogSize = 1000;
    config.lookupValue("backlogSize", backlogSize);
    if(!ok) {
        cerr << "error: Global configuration is missing options!" << endl;
        exit(3);
    }
#ifdef DEBUG
    cout << "Global configuration:" << endl;
    cout << "Web directory: " << wwwDir << endl;
    cout << "Printer config directory: " << printerConfigDir << endl;
    cout << "Storage directory: " << storageDir << endl;
#endif
}

void GlobalConfig::readPrinterConfigs() {
    printers.clear();
    if ( !exists( printerConfigDir ) ) return;
    directory_iterator end_itr; // default construction yields past-the-end
    for ( directory_iterator itr( printerConfigDir );itr != end_itr;++itr )
    {
        if ( is_regular(itr->status()) )
        {
            cout << "Printer config: " << itr->path() << endl;
            Printer *p = new Printer(itr->path().string());
            if(p->active) {
                printers.push_back(p);
            } else delete p;
        }
    }
}
void GlobalConfig::startPrinterThreads() {
    vector<Printer*>::iterator pi;
    for(pi=printers.begin();pi!=printers.end();pi++) {
        (*pi)->startThread();
    }
}
void GlobalConfig::stopPrinterThreads() {
    vector<Printer*>::iterator pi;
    for(pi=printers.begin();pi!=printers.end();pi++) {
        (*pi)->stopThread();
    }
}
Printer *GlobalConfig::findPrinterSlug(const std::string& slug) {
    for(vector<Printer*>::iterator it=printers.begin();it!=printers.end();it++) {
        Printer *p = *it;
        if(p->slugName == slug) return p;
    }
    return NULL;
}