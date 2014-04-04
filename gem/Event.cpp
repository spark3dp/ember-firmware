/*
 * File:   Event.cpp
 * Author: Richard Greene
 * Implements an event that may be subscribed to.
 * Created on March 26, 2014, 5:43 PM
 */

#include <Event.h>
#include <unistd.h>

#include <PrintEngine.h>

/// Public constructor
Event::Event(EventType eventType) :
_numBytes(0)
{
    switch(eventType)
    {
        // hardware interrupts all handled the same way
        case ButtonInterrupt:
        case MotorInterrupt:
        case DoorInterrupt:
            _inFlags = EPOLLPRI | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLPRI;
            _numBytes = 1;
            break;
            
        // timerfd expirations all handled the same way    
        case PrintEngineDelayEnd:
        case MotorTimeout:
        case PrintEnginePulse:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(uint64_t);
            break;
            
        // FIFO events handled differently depending on the data they contain
        case PrinterStatusUpdate:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(PrinterStatus);
            break;
    
        // the following all TBD
        case UICommand:
    
        case UIRequest:
    
        case USBDrive:
        
        case Error:
            
            break;
            
        default:
            //TODO: handle "impossible" case
            break;
    }
    if(_numBytes > 0)
        _data = new unsigned char[_numBytes];
}

/// Closes the file that signals the event and deletes the data buffer
Event::~Event()
{
    close(_fileDescriptor);
    delete [] _data;
}

Subscription::Subscription(EventType type,  CallbackInterface* pObject) :
_type(type),
_pObject(pObject)      
{
}

void Subscription::Call(EventType type, void* data)
{
    _pObject->callback(type, data);
}