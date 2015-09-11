//  File:   IResource.h
//  Interface to file based system resources for use with epoll
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
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

#ifndef IRESOURCE_H
#define	IRESOURCE_H

#include <vector>
#include <stdint.h>

#include "EventData.h"

typedef std::vector<EventData> EventDataVec;

class IResource
{

public:

    virtual ~IResource() {}

    // Returns one "event's" worth of data from the resource
    // The return type is an array of buffers to accommodate the situation where
    // the resource contains multiple messages that a client needs to handle
    // individually per event
    // In general, each data buffer does not contain a new-line or other
    // termination character
    virtual EventDataVec Read() = 0;

    // Returns the epoll event types applicable to this resource
    virtual uint32_t GetEventTypes() const = 0;

    // Returns the file descriptor used to read from this resource
    virtual int GetFileDescriptor() const = 0;

    // Compares the specified events with the events of concern to the resource
    // Returns true if the resource is concerned with the specified events and 
    // false otherwise
    virtual bool QualifyEvents(uint32_t events) const = 0;
    
};


#endif    // IRESOURCE_H
