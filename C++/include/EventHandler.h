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
#include "Command.h"

class EventHandler : public ICommandTarget, public ICallback
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
    void Handle(Command command);
    void HandleError(ErrorCode code, bool fatal, const char* str, int value) {}
    void Callback(EventType eventType, void* data);


private:    
    SubscriptionVec _subscriptions[MaxEventTypes];
    int _epollFd;
    std::map<int, std::pair<EventType, IResource*> > _resources;
    bool _exit; // Flag that determines if event loop will return on next iteration
};


#endif	/* EVENTHANDLER_H */

