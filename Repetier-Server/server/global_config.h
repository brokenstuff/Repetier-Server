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


#ifndef __Repetier_Server__global_config__
#define __Repetier_Server__global_config__

#include <iostream>
#include "printer.h"
#include <vector.h>

/**
This class contains in first instance the variables from repetier-server.con.
All variables can be read threadsafe through the getter functions.
*/
class GlobalConfig {
    libconfig::Config config;
    std::string wwwDir; ///< Website root
    std::string printerConfigDir; ///< Printer config directory
    std::string storageDir; ///< Data storage directory.
    std::string languageDir; ///< Dir with translation text.
    std::string ports; ///< Ports the server should listen to.
    std::string defaultLanguage; ///< Default language if no language is detected
    std::vector<Printer*> printers;
    int backlogSize;
public:
    inline const std::string& getWebsiteRoot() {return wwwDir;}
    inline const std::string& getPrinterConfigDir() {return printerConfigDir;}
    inline const std::string& getStorageDirectory() {return storageDir;}
    inline const int getBacklogSize() {return backlogSize;}
    inline const std::string& getPorts() {return ports;}
    inline const std::string& getLanguageDir() {return languageDir;}
    inline const std::string& getDefaultLanguage() {return defaultLanguage;}
    /** Load the global configuration file ans set variables accordingly. */
    GlobalConfig(std::string filename);
    /** Walk through all printer configuration files and load them. */
    void readPrinterConfigs();
    /** Start a thread for each printer to watch. */
    void startPrinterThreads();
    /** Stop all running printer threads. */
    void stopPrinterThreads();
    /** Find a printer by its slug name 
     @param slug slug name of the printer you search
     @return found printer or 0 if printer does not exist.
     */
    Printer *findPrinterSlug(const std::string& slug);
    inline std::vector<Printer*> &getPrinterList() {return printers;}
};
extern GlobalConfig *gconfig;
#endif /* defined(__Repetier_Server__global_config__) */
