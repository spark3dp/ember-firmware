/* 
 * File:   Logger.h
 * Author: Richard Greene
 * 
 * Defines a class that can log events to which it's subscribed.
 *
 * Created on April 22, 2014, 9:03 PM
 */

#include <Event.h>
#include <syslog.h>
#include <errno.h> 

#ifndef LOGGER_H
#define	LOGGER_H

class Logger : public ICallback
{  
public:
    virtual void Callback(EventType eventType, void*);
    // TODO: take a format string and varg list (...))
    static void LogError(int priority, int errnum, const char* msg);
    static void LogError(int priority, int errnum, const char* format, 
                         int value);
};

#endif	/* LOGGER_H */

