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
/// Note: in /etc/rsyslog.conf, if this default line:
///     *.*;auth,authpriv.none               -/var/log/syslog
/// is changed to 
///     *.info;auth,authpriv.none            -/var/log/syslog
/// then messages with priority of LOG_DEBUG will not be stored
/// that line would need to be restored to its default
/// in order to see items logged by this method in a release build
void Logger::Callback(EventType eventType, const EventData& data)
{
    int priority = LOG_DEBUG; 
#ifdef DEBUG
    priority = LOG_INFO;
#endif    
    
    PrinterStatus ps;

    switch(eventType)
    {
        case PrinterStatusUpdate:
            ps = data.Get<PrinterStatus>();
            // only log state entering and leaving
            if(ps._change == Entering ||
               ps._change == Leaving)
            {
                const char* substate = ps._UISubState == NoUISubState ?
                                       "" : SUBSTATE_NAME(ps._UISubState);
                syslog(priority, LOG_STATUS_FORMAT, 
                                 ps._change == Entering ? ENTERING : LEAVING,
                                 STATE_NAME(ps._state), substate);
            }
            break;
            
        case MotorInterrupt:
            syslog(priority, LOG_MOTOR_EVENT, data.Get<unsigned char>());
            break;
            
        case ButtonInterrupt:
            syslog(priority, LOG_BUTTON_EVENT, data.Get<unsigned char>());           
            break;

        case DoorInterrupt:
            syslog(priority, LOG_DOOR_EVENT, data.Get<char>());            
            break;

        case Keyboard:
            syslog(priority, LOG_KEYBOARD_INPUT, data.Get<std::string>().c_str());
            break;
            
        case UICommand:
            syslog(priority, LOG_UI_COMMAND, data.Get<std::string>().c_str());
            break;            

        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), eventType);
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
    int len = strlen(msg); 
    if(len > MAX_ERROR_MSG_LEN)
    {
        // break up large messages into smaller ones, 
        // so that syslog won't truncate them
        do
        {
            buf[MAX_ERROR_MSG_LEN - 1] = 0;
            strncpy(buf, msg, MAX_ERROR_MSG_LEN - 1);
            syslog(priority, "%s", buf);
            msg += MAX_ERROR_MSG_LEN - 1;
            len -= MAX_ERROR_MSG_LEN - 1;
        }
        while(len > 0);
    }
    else
        syslog(priority, "%s", msg);
}