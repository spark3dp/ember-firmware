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

/// Constructor increases default priority of messages, if this is a debug build.
/// Note: in /etc/systemd/journald.conf, with these settings:
///     MaxLevelStore=info
///     MaxLevelSyslog=info
/// then messages with priority of LOG_DEBUG will not be stored
/// those settings would need to be changed to =debug (or simply cmmented out)
/// in order to see log items using the default priority in a release buid
Logger::Logger() :
_defaultPriority(LOG_DEBUG) // not shown when 
{
#ifdef DEBUG
    _defaultPriority = LOG_INFO;
#endif    
}
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
                syslog(_defaultPriority, LOG_STATUS_FORMAT, pPS->_state);
            }
            break;
            
        case MotorInterrupt:
            syslog(_defaultPriority, LOG_MOTOR_EVENT, *status);
            break;
            
        case ButtonInterrupt:
            syslog(_defaultPriority, LOG_BUTTON_EVENT, *status);           
            break;

        case DoorInterrupt:
            syslog(_defaultPriority, LOG_DOOR_EVENT, *(char*)data);            
            break;

        case Keyboard:
            syslog(_defaultPriority, LOG_KEYBOARD_INPUT, (char*)data);
            break;
            
        case UICommand:
            syslog(_defaultPriority, LOG_UI_COMMAND, *(int*)data);
            break;            

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
 
char buf[MAX_ERROR_MSG_LEN];

/// Log the given error and send it out to stderr
char* Logger::LogError(int priority, int errnum, const char* msg)
{
    syslog(priority, LOG_ERROR_FORMAT, msg, strerror(errnum));
    sprintf(buf, ERROR_FORMAT, msg, strerror(errnum));
    std::cerr << buf << std::endl;
    return buf;
}

/// Format and log the given error with a numeric value and send it to stderr
char* Logger::LogError(int priority, int errnum, const char* format, 
                      int value)
{
    sprintf(buf, format, value);
    return LogError(priority, errnum, buf);
}

/// Format and log the given error with a string and send it to stderr
char* Logger::LogError(int priority, int errnum, const char* format, 
                      const char* str)
{
    sprintf(buf, format, str);
    return LogError(priority, errnum, buf);
}

