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


#ifndef __Repetier_Server__PrinterSerial__
#define __Repetier_Server__PrinterSerial__

#include <iostream>
#include <list>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/cstdint.hpp>
using namespace boost;

class Printer;

#define READ_BUFFER_SIZE 1024

class PrinterSerialPort:public boost::asio::serial_port {
public:
    inline PrinterSerialPort(boost::asio::io_service &io):boost::asio::serial_port(io) {}
    void set_baudrate(int baud);
    void debugTermios();
};
/** Handles the connection with one printer.
 */
class PrinterSerial {
    boost::asio::io_service io; ///< Io service object
                                //boost::asio::serial_port port; ///< Serial port object
    PrinterSerialPort port; ///< Serial port object
    boost::asio::serial_port_base::flow_control flowControl;
    boost::asio::serial_port_base::parity parity;
    boost::asio::serial_port_base::stop_bits stopBits;
    boost::asio::serial_port_base::baud_rate baudrate;
    boost::asio::serial_port_base::character_size characterSize;
    boost::thread backgroundThread; ///< Thread that runs read/write operations
    bool open; ///< True if port open
    bool error; ///< Error flag
    mutable boost::mutex errorMutex; ///< Mutex for access to error
    /// Data are queued here before they go in writeBuffer
    std::vector<char> writeQueue;
    boost::shared_array<char> writeBuffer; ///< Data being written
    size_t writeBufferSize; ///< Size of writeBuffer
    boost::mutex writeQueueMutex; ///< Mutex for access to writeQueue
    char readBuffer[READ_BUFFER_SIZE]; ///< data being read
    /// Read complete callback
    boost::function<void (const char*, size_t)> callback;
    std::string readString;
    
    
    
    Printer *printer;
    
    void doRead();
    void readEnd(const boost::system::error_code& error,size_t bytes_transferred);
    void doWrite();
    void writeEnd(const boost::system::error_code& error);
    void doClose();
public:
    
    PrinterSerial(Printer &prt);
    ~PrinterSerial();
    
    // Returns true if printer is connected
    bool isConnected();
    // Tries to connect to printer
    bool tryConnect();
    // Close connection
    void close();
    
    bool isOpen();
    bool errorStatus() const;
    void setErrorStatus(bool e);
    /** Send a string to serial output. */
    void writeString(const std::string& s);
    /** Write some bytes to serial output. */
    void writeBytes(const uint8_t* data,size_t len);
    /** Send reset to the printer by toggling DTR line. */
    void resetPrinter();
};

#endif /* defined(__Repetier_Server__PrinterSerial__) */
