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
_pollFd(epoll_create(MaxEventTypes))
{
    // initialize array of Events with file descriptors set to "empty" values
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
    {
        _pEvents[et] = new Event((EventType)et);
    } 
    
    if (_pollFd < 0) 
        throw std::runtime_error(ErrorMessage::Format(EpollCreate, errno));
}

/*
 * Destructor, closes epoll file descriptor
 */
EventHandler::~EventHandler()
{
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
        delete _pEvents[et];  
    
    close(_pollFd);
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
    _resources[eventType] = pResource;
    _fdMap[event.data.fd] = eventType;
    epoll_ctl(_pollFd, EPOLL_CTL_ADD, pResource->GetFileDescriptor(), &event);
}

/*
 * Add a subscriber to a given event type
 */
void EventHandler::Subscribe(EventType eventType, ICallback* pObject)
{
    _pEvents[eventType]->_subscriptions.push_back(pObject);
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
        numFDs = epoll_wait(_pollFd, events, MaxEventTypes, timeout);
        
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
                int fd = events[n].data.fd;
                EventType et = _fdMap[fd];
                
                // Qualify the event
                // The event loop only cares about specific types of events - it
                // might want to ignore some types of events picked up by epoll
                // (EPOLLERR, etc.)
                if(!(events[n].events & _pEvents[et]->_outFlags))
                    continue;
                
                // Read the data associated with the event
                IResource* resource = _resources[et];
                ResourceBufferVec buffers = resource->Read();
                for (ResourceBufferVec::iterator it = buffers.begin();
                        it != buffers.end(); it++)
                {
                    ResourceBuffer buffer = *it;
                    char data[buffer.size() + 1]; // add one for null terminator
                    std::copy(buffer.begin(), buffer.end(), data);
                    data[buffer.size()] = '\0';
                    _pEvents[et]->CallSubscribers(et, data);
                }
            } 
        }      
#ifdef DEBUG
        if(!doForever && --_numIterations < 0)
            keepGoing = false;
#endif        
    }
}
