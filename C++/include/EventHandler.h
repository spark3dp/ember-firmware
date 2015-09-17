//  File:   EventHandler.h
//  Defines a handler for events detected on file descriptors
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

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
    // for testing only
    void Begin(int numIterations);
#endif    
    void AddEvent(EventType eventType, IResource* pResource);
    void Handle(Command command);
    void HandleError(ErrorCode code, bool fatal, const char* str, int value) {}
    void Callback(EventType eventType, const EventData& data);


private:    
    SubscriptionVec _subscriptions[MaxEventTypes];
    int _epollFd;
    std::map<int, std::pair<EventType, IResource*> > _resources;
    // exit flag determines if event loop will return on next iteration
    bool _exit; 
};


#endif    // EVENTHANDLER_H

