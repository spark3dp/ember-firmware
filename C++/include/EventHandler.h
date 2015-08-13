/* 
 * File:   EventHandler.h
 * Author: Richard Greene
 *
 * Created on March 31, 2014, 1:48 PM
 */

#ifndef EVENTHANDLER_H
#define	EVENTHANDLER_H

#include <map>

#include <Event.h>
#include <Hardware.h>

#include "IResource.h"

class EventHandler
{
public:
    EventHandler();
    ~EventHandler(); 
    void Subscribe(EventType eventType, ICallback* pObject);
    void Begin();
#ifdef DEBUG  
    void Begin(int numIterations);
#endif    
    void AddEvent(EventType eventType, IResource* pResource);
    
private:    
    Event* _pEvents[MaxEventTypes];
    int _pollFd;
    std::map<EventType, IResource*> _resources;
    std::map<int, EventType> _fdMap;
};


#endif	/* EVENTHANDLER_H */

