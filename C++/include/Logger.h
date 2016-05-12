//  File:   Logger.h
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

#ifndef LOGGER_H
#define	LOGGER_H

#include <syslog.h>
#include <errno.h>
#include <limits.h>

#include <ErrorMessage.h>
#include "EventType.h"
#include "ICallback.h"

// Static methods providing logging services to all components
class Logger : public ICallback
{  
public:
    virtual void Callback(EventType eventType, const EventData& data);
    static char* LogError(int priority, ErrorCode errorCode);
    static char* LogError(int priority, int errnum, ErrorCode errorCode);
    static char* LogError(int priority, int errnum, ErrorCode errorCode, int value);
    static char* LogError(int priority, int errnum, ErrorCode errorCode, 
                   const char* str);
    static char* LogError(int priority, int errnum, ErrorCode errorCode, 
                   const std::string& str);
    static bool HandleError(ErrorCode code, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX);
    static void LogMessage(int priority, const char* msg);

private:   
    static char* LogError(int priority, int errnum, const char* msg);
};

#endif    // LOGGER_H

