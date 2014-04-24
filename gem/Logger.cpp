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

/// Handle the events we wish to log
void Logger::Callback(EventType eventType, void* data)
{
    PrinterStatus* pPS;
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

/// Format and log the given error and send it out to stderr
void Logger::LogError(int priority, int errnum, const char* format, 
                      int value)
{
    sprintf(_buf, format, value);
    LogError(priority, errnum, _buf);
}

