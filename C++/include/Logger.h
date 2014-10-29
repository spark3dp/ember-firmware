/* 
 * File:   Logger.h
 * Author: Richard Greene
 * 
 * Defines a class that can log events to which it's subscribed.
 *
 * Created on April 22, 2014, 9:03 PM
 */


#include <syslog.h>
#include <errno.h>
#include <limits.h>

#include <Event.h>
#include <ErrorMessage.h>

#ifndef LOGGER_H
#define	LOGGER_H

#define LOGGER (Logger::Instance())

// ABC defining the interface to a class that handles errors.
class IErrorHandler
{
public:
    virtual void HandleError(ErrorCode code, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX) = 0;
};



/// Singleton providing logging services to all components
class Logger : public ICallback, public IErrorHandler
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
    void HandleError(ErrorCode code, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX);
    void LogMessage(int priority, const char* msg);

private:   
    Logger() {};
    Logger(Logger const&);
    Logger& operator=(Logger const&);
    ~Logger() {};
};

#endif	/* LOGGER_H */

