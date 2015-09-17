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

#define LOGGER (Logger::Instance())

// ABC defining the interface to a class that handles errors.
class IErrorHandler
{
public:
    virtual void HandleError(ErrorCode code, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX) = 0;
};



// Singleton providing logging services to all components
class Logger : public ICallback, public IErrorHandler
{  
public:
    static Logger& Instance();

    virtual void Callback(EventType eventType, const EventData& data);
    char* LogError(int priority, int errnum, const char* msg);
    char* LogError(int priority, int errnum, const char* format, int value);
    char* LogError(int priority, int errnum, const char* format, 
                   const char* str);
    void HandleError(ErrorCode code, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX);
    void LogMessage(int priority, const char* msg);

private:   
    Logger() {};
    Logger(Logger const&);
    Logger& operator=(Logger const&);
    ~Logger() {};
};

#endif    // LOGGER_H

