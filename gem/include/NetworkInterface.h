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

#include <Event.h>

/// Defines the interface to the Internet
class NetworkInterface: public ICallback
{
public:   
    NetworkInterface();
    
private:
    int _statusWriteFd;
    
    void Callback(EventType eventType, void* data);
};


#endif	/* NETWORKINTERFACE_H */

