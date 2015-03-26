/*
 * File:   Event.cpp
 * Author: Richard Greene
 * Implements an event that may be subscribed to.
 * Created on March 26, 2014, 5:43 PM
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <Event.h>
#include <PrinterStatus.h>
#include <MessageStrings.h>
#include <Logger.h>
#include <Commands.h>

/// Public constructor, defines specifics needed to handle each type of event
Event::Event(EventType eventType) :
_numBytes(0),
_isHardwareInterrupt(false),
_pI2CDevice(NULL),
_handleAllAvailableInput(false),
_data(NULL)
{
    switch(eventType)
    {
        // hardware interrupts handled in similar ways
        case ButtonInterrupt:
        case MotorInterrupt:
        case DoorInterrupt:
        case RotationInterrupt:
            _inFlags = EPOLLPRI | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLPRI;
            _numBytes = 1;
            _isHardwareInterrupt = true;
            break;
            
        // timerfd expirations all handled the same way    
        case ExposureEnd:
        case MotorTimeout:
        case TemperatureTimer:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(uint64_t); 
            break;
            
        // FIFO events may be handled differently depending on the data they contain
        case PrinterStatusUpdate:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(PrinterStatus);
            _handleAllAvailableInput = true;
            break;
            
        case UICommand:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            // string commands use a separate buffer
            break;
            
        case Keyboard:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLPRI;	
            _outFlags = EPOLLIN;
            _numBytes = 256;  // string commands, which should be < 20 chars
            break;                        
            
        default:
            // "impossible" case
            LOGGER.LogError(LOG_ERR, errno,ERR_MSG(UnknownEventType), eventType);
            exit(-1);
            break;
    }
    if(_numBytes > 0)
    {
        _data = new unsigned char[_numBytes];
        memset(_data, 0, _numBytes);
    }
}

/// Closes the file that signals the event and deletes the data buffer.
Event::~Event()
{
    if(_fileDescriptor >= 0)
        close(_fileDescriptor);
    if(_numBytes > 0)
        delete [] _data;
}

// Calls all subscribers to this event.
void Event::CallSubscribers(EventType type, void* data)
{
    int numSubscribers = _subscriptions.size();
    for(int i = 0; i < numSubscribers; i++)
        _subscriptions[i]->Callback(type, data);
}

void ICallback::HandleImpossibleCase(EventType eventType)
{
    LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), eventType);
}
