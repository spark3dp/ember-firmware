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
#include <MessageStrings.h>
#include <Shared.h>

#define MAX_ERROR_MSG_LEN (1024)

/// Gets the Logger singleton
Logger& Logger::Instance()
{
    static Logger logger;

    return logger;
}

/// Handle the events we wish to log
/// Increases priority of messages, if this is a debug build.
/// Note: in /etc/systemd/journald.conf, with these settings:
///     MaxLevelStore=info
///     MaxLevelSyslog=info
/// then messages with priority of LOG_DEBUG will not be stored
/// those settings would need to be changed to =debug (or simply commented out)
/// in order to see log items using the default priority in a release build
void Logger::Callback(EventType eventType, void* data)
{
    int priority = LOG_DEBUG; 
#ifdef DEBUG
    priority = LOG_INFO;
#endif    
    
    PrinterStatus* pPS;
    unsigned char* status = (unsigned char*)data; 
    switch(eventType)
    {
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            // only log state entering and leaving
            if(pPS->_change == Entering ||
               pPS->_change == Leaving)
            {
                const char* substate = pPS->_UISubState == NoUISubState ?
                                       "" : SUBSTATE_NAME(pPS->_UISubState);
                syslog(priority, LOG_STATUS_FORMAT, 
                                 pPS->_change == Entering ? ENTERING : LEAVING,
                                 STATE_NAME(pPS->_state), substate);
            }
            break;
            
        case MotorInterrupt:
            syslog(priority, LOG_MOTOR_EVENT, *status);
            break;
            
        case ButtonInterrupt:
            syslog(priority, LOG_BUTTON_EVENT, *status);           
            break;

        case DoorInterrupt:
            syslog(priority, LOG_DOOR_EVENT, *(char*)data);            
            break;

        case Keyboard:
            syslog(priority, LOG_KEYBOARD_INPUT, (char*)data);
            break;
            
        case UICommand:
            syslog(priority, LOG_UI_COMMAND, (char*)data);
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

/// Implements IErrorHandler by simply logging the given error
void Logger::HandleError(ErrorCode code, bool fatal, const char* str, 
                                                                      int value)
{
    const char* baseMsg = ERR_MSG(code);
    if(str != NULL)
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, baseMsg, str);
    else if (value != INT_MAX)
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, baseMsg, value);
    else
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, baseMsg);
}

/// Log a message with the given priority
void Logger::LogMessage(int priority, const char* msg)
{
    syslog(priority, "%s", msg);
}