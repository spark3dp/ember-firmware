/*
 * File:   EventHandler.cpp
 * Author: Richard Greene and Jason Lefley
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>

#include "EventHandler.h"
#include "ErrorMessage.h"
#include "Logger.h"

/*
 * Constructor, initializes epoll instance according to number of events
 */
EventHandler::EventHandler() :
_epollFd(epoll_create(MaxEventTypes))
{
    if (_epollFd < 0) 
        throw std::runtime_error(ErrorMessage::Format(EpollCreate, errno));
}

/*
 * Destructor, closes epoll file descriptor
 */
EventHandler::~EventHandler()
{
    close(_epollFd);
}

/*
 * Add an event with an associated resource
 * The specified event type will arise when activity is detected on the
 * specified resource
 */
void EventHandler::AddEvent(EventType eventType, IResource* pResource)
{
    epoll_event event;
    event.events = pResource->GetEventTypes();
    event.data.fd = pResource->GetFileDescriptor();
    _resources[event.data.fd] = std::make_pair(eventType, pResource);
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, event.data.fd, &event);
}

/*
 * Add a subscriber to a given event type
 */
void EventHandler::Subscribe(EventType eventType, ICallback* pObject)
{
    _subscriptions[eventType].push_back(pObject);
}

#ifdef DEBUG
int _numIterations = 0;
// Debug only version of Begin allows unit testing with a finite number of iterations
void EventHandler::Begin(int numIterations)
{
    _numIterations = numIterations;  
    Begin();
}
#endif 
    
/// Begin handling events, in an infinite loop.
void EventHandler::Begin()
{   
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
        int timeout = -1;
#ifdef DEBUG
        // use 10 ms timeout for unit testing 
        if(!doForever)
            timeout = 10;
#endif              
        // Do a blocking epoll_wait, there's nothing to do until it returns
        numFDs = epoll_wait(_epollFd, events, MaxEventTypes, timeout);
        
        if(numFDs)
        {
            // numFDs file descriptors are ready for the requested IO

            // Handle possible error condition
            if(numFDs < 0 && errno != EINTR)
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
                
                ResourceBufferVec buffers = resource->Read();
                for (ResourceBufferVec::iterator bufIt = buffers.begin();
                        bufIt != buffers.end(); bufIt++)
                {
                    ResourceBuffer buffer = *bufIt;
                    char data[buffer.size() + 1]; // add one for null terminator
                    std::copy(buffer.begin(), buffer.end(), data);
                    data[buffer.size()] = '\0';
                    
                    // Call each subscriber
                    SubscriptionVec subscriptions = _subscriptions[eventType];

                    for (SubscriptionVec::iterator it = subscriptions.begin();
                            it != subscriptions.end(); it++)
                        (*it)->Callback(eventType, data);
                }
            } 
        }      
#ifdef DEBUG
        if(!doForever && --_numIterations < 0)
            keepGoing = false;
#endif        
    }
}
