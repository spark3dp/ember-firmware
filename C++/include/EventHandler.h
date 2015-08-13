/* 
 * File:   EventHandler.h
 * Author: Richard Greene
 *
 * Created on March 31, 2014, 1:48 PM
 */

#ifndef EVENTHANDLER_H
#define	EVENTHANDLER_H

#include <map>

#include <Event.h>
#include <Hardware.h>

#include "IResource.h"

class EventHandler
{
public:
    EventHandler();
    ~EventHandler(); 
//    void SetFileDescriptor(EventType eventType, int fd);
//    void SetI2CDevice(EventType eventType, I2C_Device* pDevice,
//                                unsigned char statusReg);
    void Subscribe(EventType eventType, ICallback* pObject);
    void Begin();
#ifdef DEBUG  
    void Begin(int numIterations);
#endif    
    void AddEvent(EventType eventType, IResource* pResource);
    
private:    
    Event* _pEvents[MaxEventTypes];
    int _pollFd;
//    int _commandReadFd;
//    int _commandWriteFd;
    std::map<EventType, IResource*> _resources;
    std::map<int, EventType> _fdMap;
    
//    int GetInterruptDescriptor(EventType eventType);
//    void UnexportPins();  
//    int GetInputPinFor(EventType et);
};


#endif	/* EVENTHANDLER_H */

