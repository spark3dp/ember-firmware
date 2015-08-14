/* 
 * File:   EventHandler.h
 * Author: Richard Greene
 *
 * Created on March 31, 2014, 1:48 PM
 */

#ifndef EVENTHANDLER_H
#define	EVENTHANDLER_H

#include <map>
#include <vector>

#include "IResource.h"
#include "EventType.h"
#include "ICallback.h"

class EventHandler
{
typedef std::vector<ICallback*> SubscriptionVec;

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
    SubscriptionVec _subscriptions[MaxEventTypes];
    int _epollFd;
    std::map<int, std::pair<EventType, IResource*> > _resources;
};


#endif	/* EVENTHANDLER_H */

