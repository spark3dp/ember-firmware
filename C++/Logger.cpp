//  File:   Logger.cpp
//  Defines a class that can log events, errors, and other messages
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <syslog.h>
#include <iostream>
#include <stdio.h>

#include <Logger.h>
#include <PrinterStatus.h>
#include <MessageStrings.h>
#include <Shared.h>

constexpr int MAX_ERROR_MSG_LEN = 1024;

// Handle the events we wish to log
// Increases priority of messages, if this is a debug build.
// Note: in /etc/rsyslog.conf, if this default line:
//     *.*;auth,authpriv.none               -/var/log/syslog
// is changed to 
//     *.info;auth,authpriv.none            -/var/log/syslog
// then messages with priority of LOG_DEBUG will not be stored
// that line would need to be restored to its default
// in order to see items logged by this method in a release build
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
            if (ps._change == Entering || ps._change == Leaving)
            {
                if(ps._state != ExposingState || ps._change != Entering)
                {
                    const char* substate = (ps._UISubState == NoUISubState) ?
                           "" : PrinterStatus::GetSubStateName(ps._UISubState);
                    syslog(priority, LOG_STATUS_FORMAT, 
                           ps._change == Entering ? ENTERING : LEAVING,
                           PrinterStatus::GetStateName(ps._state), substate);
                }
                else
                {
                    // log extra info when starting each exposure
                    syslog(priority, LOG_EXPOSING_FORMAT, 
                           ps._change == Entering ? ENTERING : LEAVING,
                           PrinterStatus::GetStateName(ps._state), 
                            ps._currentLayer, ps._numLayers,
                            ps._estimatedSecondsRemaining);
                }
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
            syslog(priority, LOG_KEYBOARD_INPUT, 
                                            data.Get<std::string>().c_str());
            break;
            
        case UICommand:
            syslog(priority, LOG_UI_COMMAND, data.Get<std::string>().c_str());
            break;            

        default:
            LogError(LOG_WARNING, errno, UnexpectedEvent, eventType);
            break;
    }
}
 
char buf[MAX_ERROR_MSG_LEN];

// Log the given error message and send it out to stderr.
char* Logger::LogError(int priority, int errnum, const char* msg)
{
    syslog(priority, LOG_ERROR_FORMAT, msg, strerror(errnum));
    sprintf(buf, ERROR_FORMAT, msg, strerror(errnum));
    std::cerr << buf << std::endl;
    return buf;
}

// Log the given error message and send it out to stderr.
char* Logger::LogError(int priority, ErrorCode errorCode)
{
    const char* message = ErrorMessage::GetMessage(errorCode);
    syslog(priority, message);
    std::cerr << message << std::endl;
    std::strcpy(buf, message);
    return buf;
}

// Log the message for the given error code and send it out to stderr.
char* Logger::LogError(int priority, int errnum, ErrorCode errorCode)
{
    return LogError(priority, errnum, ErrorMessage::GetMessage(errorCode));
}

// Format and log the given error with a numeric value and send it to stderr
char* Logger::LogError(int priority, int errnum, ErrorCode errorCode, 
                       int value)
{
    sprintf(buf, ErrorMessage::GetMessage(errorCode), value);
    return LogError(priority, errnum, buf);
}

// Format and log the given error with a string and send it to stderr
char* Logger::LogError(int priority, int errnum, ErrorCode errorCode, 
                      const char* str)
{
    sprintf(buf, ErrorMessage::GetMessage(errorCode), str);
    return LogError(priority, errnum, buf);
}

char* Logger::LogError(int priority, int errnum, ErrorCode errorCode,
                       const std::string& str)
{
    LogError(priority, errnum, errorCode, str.c_str());
}

// Implements IErrorHandler by simply logging the given error.
// Always returns false for convenience.
bool Logger::HandleError(ErrorCode code, bool fatal, const char* str, 
                                                                      int value)
{
    if (str != NULL)
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, code, str);
    else if (value != INT_MAX)
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, code, value);
    else
        LogError(fatal ? LOG_ERR : LOG_WARNING, errno, code);
    return false;
}

// Log a message with the given priority
void Logger::LogMessage(int priority, const char* msg)
{
    int len = strlen(msg); 
    if (len > MAX_ERROR_MSG_LEN)
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