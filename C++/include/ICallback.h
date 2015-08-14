/* 
 * File:   ICallback.h
 * Author: Richard Greene and Jason Lefley
 *
 * Interface to class that supports callbacks from EventHandler
 * 
 * Created on August 14, 2015, 10:05 AM
 */

#ifndef ICALLBACK_H
#define	ICALLBACK_H

#include "EventType.h"

class ICallback
{
public:
    virtual void Callback(EventType eventType, void* data) = 0;
};

#endif	/* ICALLBACK_H */

