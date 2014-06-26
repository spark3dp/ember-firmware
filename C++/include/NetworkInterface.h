/* 
 * File:   NetworkInterface.h
 * Author: Richard Greene
 *
 * Connects the Internet to the EventHandler.
 * 
 * Created on May 14, 2014, 5:45 PM
 */

#ifndef NETWORKINTERFACE_H
#define	NETWORKINTERFACE_H

#include <string>

#include <Event.h>
#include <PrinterStatus.h>
#include <Commands.h>


/// Defines the interface to the Internet
class NetworkInterface: public ICallback
{
public:   
    NetworkInterface();
    ~NetworkInterface();
        
private:
    int _statusPushFd;
    int _commandResponseFd;
    std::string _statusJSON;
    
    void Callback(EventType eventType, void* data);
    void HandleWebCommand(const char* cmd);
    void SaveCurrentStatus(PrinterStatus* pStatus);
    void SendCurrentStatus(int fileDescriptor);
};


#endif	/* NETWORKINTERFACE_H */

