//  File: EventQueue.h
//  FIFO queue to hold events for deferred handling
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

// Capacity of queue in terms of how many events can be stored
// Needs to be a power of 2 so compiler can use AND to implement modulo
#define EVENT_QUEUE_LENGTH 16

#include <stdint.h>

#include "EventData.h"
#include "Status.h"

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();

    Status Add(SM_EVENT_CODE_TYPE eventCode, EventData eventData);
    void Remove(SM_EVENT_CODE_TYPE& eventCode, EventData& eventData);
    void Clear();
    bool IsEmpty() const;

private:
    EventQueue(const EventQueue&);

private:
    uint8_t head = 0;
    uint8_t tail = 0;
    EventData eventDataBuffer[EVENT_QUEUE_LENGTH];
    SM_EVENT_CODE_TYPE eventCodeBuffer[EVENT_QUEUE_LENGTH];

};

#endif  // EVENTQUEUE_H
