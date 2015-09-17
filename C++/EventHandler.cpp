//  File:   EventHandler.cpp
//  Implements a handler for events detected on file descriptors
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

#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>
#include <signal.h>

#include "EventHandler.h"
#include "ErrorMessage.h"
#include "Logger.h"

// Constructor, initializes epoll instance according to number of events
EventHandler::EventHandler() :
_epollFd(epoll_create(MaxEventTypes)),
_exit(false)
{
    if (_epollFd < 0) 
        throw std::runtime_error(ErrorMessage::Format(EpollCreate, errno));
}

// Destructor, closes epoll file descriptor
EventHandler::~EventHandler()
{
    close(_epollFd);
}

// Add an event with an associated resource
// The specified event type will arise when activity is detected on the
// specified resource
void EventHandler::AddEvent(EventType eventType, IResource* pResource)
{
    epoll_event event;
    event.events = pResource->GetEventTypes();
    event.data.fd = pResource->GetFileDescriptor();
    _resources[event.data.fd] = std::make_pair(eventType, pResource);
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, event.data.fd, &event);
}

// Add a subscriber to a given event type
void EventHandler::Subscribe(EventType eventType, ICallback* pObject)
{
    _subscriptions[eventType].push_back(pObject);
}

#ifdef DEBUG
int _numIterations = 0;
// Debug only version of Begin allows unit testing with a finite number of 
// iterations
void EventHandler::Begin(int numIterations)
{
    _numIterations = numIterations;  
    Begin();
}
#endif 
    
// Begin handling events, in an infinite loop.
void EventHandler::Begin()
{
    _exit = false;

#ifdef DEBUG
    // do repeatedly if _numIterations is zero 
    bool doForever = _numIterations == 0;
#endif    

    // epoll event structure for receiving events
    struct epoll_event events[MaxEventTypes];
   
    bool keepGoing = true;
    int numFDs = 0;
    
    // Start event loop, waiting for events, and calling subscribers when loop
    // receives events
    while(keepGoing)
    {
        if (_exit)
            return;

        int timeout = -1;
#ifdef DEBUG
        // use 10 ms timeout for unit testing 
        if (!doForever)
            timeout = 10;
#endif              
        // Do a blocking epoll_wait, there's nothing to do until it returns
        numFDs = epoll_wait(_epollFd, events, MaxEventTypes, timeout);
        
        if (numFDs)
        {
            // numFDs file descriptors are ready for the requested IO

            // Handle possible error condition
            if (numFDs < 0 && errno != EINTR)
            {
                // If this keeps repeating, it should probably be a fatal error
                LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(NegativeNumFiles), 
                                 numFDs);
            }
            
            for(int n = 0; n < numFDs; n++)
            {
                epoll_event event = events[n];
                
                // Read the data associated with the event
                IResource* resource = _resources[event.data.fd].second;
                EventType eventType = _resources[event.data.fd].first;

                // Qualify the event
                // The event loop only cares about specific types of events - it
                // might want to ignore some types of events picked up by epoll
                // (EPOLLERR, etc.)
                if (!resource->QualifyEvents(event.events))
                    continue;
                
                EventDataVec eventData = resource->Read();
                for (EventDataVec::iterator eventDataIt = eventData.begin();
                        eventDataIt != eventData.end(); eventDataIt++)
                {
                    // call each subscriber with the event data
                    SubscriptionVec subscriptions = _subscriptions[eventType];

                    for (SubscriptionVec::iterator it = subscriptions.begin();
                            it != subscriptions.end(); it++)
                        (*it)->Callback(eventType, *eventDataIt);
                }
            } 
        }      
#ifdef DEBUG
        if (!doForever && --_numIterations < 0)
            keepGoing = false;
#endif        
    }
}

void EventHandler::Handle(Command command)
{
    // PrintEngine handles all other commands
    if (command == Exit)
        _exit = true;
}

void EventHandler::Callback(EventType eventType, const EventData& data)
{
    if (eventType == Signal)
    {
        uint32_t signal = data.Get<uint32_t>();
        
        // exit on receipt of TERM and INT
        // ignore others
        if (signal == SIGINT || signal == SIGTERM)
            _exit = true;
    }
}
