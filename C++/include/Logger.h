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

#define LOGGER (Logger::Instance())

/// Singleton providing logging services to all components
class Logger : public ICallback
{  
public:
    static Logger& Instance();

    virtual void Callback(EventType eventType, void*);
    // TODO: take a format string and varg list (...) ?)
    char* LogError(int priority, int errnum, const char* msg);
    char* LogError(int priority, int errnum, const char* format, 
                          int value);
    char* LogError(int priority, int errnum, const char* format, 
                          const char* str);

private:   
    Logger() {};
    Logger(Logger const&);
    Logger& operator=(Logger const&);
    ~Logger() {};
};

#endif	/* LOGGER_H */

