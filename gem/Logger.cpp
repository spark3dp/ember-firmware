/* 
 * File:   Logger.cpp
 * Author: Richard Greene
 * 
 * Implements a class that can log events to which it's subscribed.
 *
 * Created on April 22, 2014, 9:03 PM
 */

#include <Logger.h>
#include <PrinterStatus.h>

#include <syslog.h>

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
 

#include <syslog.h>

