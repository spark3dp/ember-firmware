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
class NetworkInterface: public ICallback, public ICommandTarget
{
public:   
    NetworkInterface();
    ~NetworkInterface();
    void Handle(Command command);
    void HandleError(const char* baseMsg, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX);
        
private:
    int _statusPushFd;
    int _commandResponseFd;
    std::string _statusJSON;
    
    void Callback(EventType eventType, void* data);
    void HandleWebCommand(const char* cmd);
    void SaveCurrentStatus(PrinterStatus* pStatus);
    void SendStringToPipe(const char* str, int fileDescriptor);
};


#endif	/* NETWORKINTERFACE_H */

