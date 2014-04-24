/* 
 * File:   Logger.cpp
 * Author: Richard Greene
 * 
 * Implements a class that can log events to which it's subscribed.
 *
 * Created on April 22, 2014, 9:03 PM
 */

#include <syslog.h>
#include <iostream>
#include <stdio.h>

#include <Logger.h>
#include <PrinterStatus.h>

#define MAX_ERROR_MSG_LEN (1024)

/// Handle the events we wish to log
void Logger::Callback(EventType eventType, void* data)
{
    PrinterStatus* pPS;
    unsigned char* status = (unsigned char*)data; 
    switch(eventType)
    {
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            if(pPS->_change == Entering)
            {
                // for first pass, only log state entering events
                syslog(LOG_INFO, LOG_STATUS_FORMAT, pPS->_state);
            }
            break;
            
        case MotorInterrupt:
            syslog(LOG_INFO, LOG_MOTOR_EVENT, *status);
            break;
            
        case ButtonInterrupt:
            syslog(LOG_INFO, LOG_BUTTON_EVENT, *status);           
            break;

        case DoorInterrupt:
            syslog(LOG_INFO, LOG_DOOR_EVENT, *(char*)data);            
            break;

        case Keyboard:
            syslog(LOG_INFO, LOG_KEYBOARD_INPUT, (char*)data);
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
 
/// Log the given error and send it out to stderr
void Logger::LogError(int priority, int errnum, const char* msg)
{
    syslog(priority, LOG_ERROR_FORMAT, msg, strerror(errnum));
    
    std::cerr << msg << STDERR_FORMAT << strerror(errnum) << std::endl;
}

char buf[MAX_ERROR_MSG_LEN]; 
/// Format and log the given error and send it out to stderr
void Logger::LogError(int priority, int errnum, const char* format, 
                      int value)
{
    sprintf(buf, format, value);
    LogError(priority, errnum, buf);
}

