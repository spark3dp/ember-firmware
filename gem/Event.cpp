/*
 * File:   Event.cpp
 * Author: Richard Greene
 * Implements an event that may be subscribed to.
 * Created on March 26, 2014, 5:43 PM
 */

#include <Event.h>
#include <unistd.h>


/// Public constructor, sets member variables
/// [should it also open the file, to create the file descriptor, instead of having it passed in?]
Event::Event(EventType type, int fileDescriptor, 
             uint32_t inFlags, uint32_t outFlags) :
_type(type),
_fileDescriptor(fileDescriptor),  
_inFlags(inFlags),
_outFlags(outFlags)       
{
    
}

/// Closes the file that signals the event
Event::~Event()
{
    close(_fileDescriptor);
}

Subscription::Subscription(EventType type, EventCallback callback) :
_type(type),
_callback(callback)      
{
}
