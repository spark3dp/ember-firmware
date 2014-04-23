/* 
 * File:   Logger.h
 * Author: Richard Greene
 * 
 * Defines a class that can log events to which it's subscribed.
 *
 * Created on April 22, 2014, 9:03 PM
 */

#include <Event.h>

#ifndef LOGGER_H
#define	LOGGER_H

class Logger : public ICallback
{  
public:
    virtual void Callback(EventType eventType, void*);
};

#endif	/* LOGGER_H */

