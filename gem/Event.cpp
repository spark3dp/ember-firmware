/*
 * File:   Event.cpp
 * Author: Richard Greene
 * Implements an event that may be subscribed to.
 * Created on March 26, 2014, 5:43 PM
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Event.h>
#include <PrinterStatus.h>
#include <MessageStrings.h>

// TODO: move this to a separate utility for reporting formatted error strings
char msg[100];
char* FormatError(const char * format, int value)
{
    sprintf(msg, format, value);
    return msg;
}

long getMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/// Public constructor, defines specifics needed to handle each type of event
Event::Event(EventType eventType) :
_numBytes(0),
_isHardwareInterrupt(false),
_ignoreAllButLatest(false)        
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
            _isHardwareInterrupt = true;
            break;
            
        // timerfd expirations all handled the same way    
        case ExposureEnd:
        case MotorTimeout:
        case PrintEnginePulse:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(uint64_t); 
            break;
            
        // FIFO events may be handled differently depending on the data they contain
        case PrinterStatusUpdate:
            _inFlags = EPOLLIN | EPOLLERR | EPOLLET;	
            _outFlags = EPOLLIN;
            _numBytes = sizeof(PrinterStatus);
            _ignoreAllButLatest = true;
            break;
    
        // the following all TBD
        case UICommand:
    
        case UIRequest:
    
        case USBDrive:
        
        case Error:
            
            break;
            
        default:
            // "impossible" case
            perror(FormatError(UNKNOWN_EVENT_TYPE_ERROR, eventType));
            exit(-1);
            break;
    }
    if(_numBytes > 0)
        _data = new unsigned char[_numBytes];
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
