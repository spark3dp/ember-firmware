/* 
 * File:   EventHandler.h
 * Author: greener
 *
 * Created on March 31, 2014, 1:48 PM
 */

#ifndef EVENTHANDLER_H
#define	EVENTHANDLER_H

#include <queue>
#include <string>

#include <Event.h>
#include <Hardware.h>

class EventHandler
{
public:
    EventHandler();
    ~EventHandler(); 
    void SetFileDescriptor(EventType eventType, int fd);
    void SetI2CDevice(EventType eventType, I2C_Device* pDevice,
                                unsigned char statusReg);
    void Subscribe(EventType eventType, ICallback* pObject);
    void Begin();
#ifdef DEBUG  
    void Begin(int numIterations);
#endif    
    
private:    
    Event* _pEvents[MaxEventTypes];
    std::queue<std::string> _commands;
    
    int GetInterruptDescriptor(EventType eventType);
    void UnexportPins();  
    int GetInputPinFor(EventType et);
};


#endif	/* EVENTHANDLER_H */

