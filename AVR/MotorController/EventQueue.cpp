//  File: EventQueue.cpp
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

#include "EventQueue.h"
#include "Debug.h"

EventQueue::EventQueue()
{
}

EventQueue::~EventQueue()
{
}

// Add to end of queue
Status EventQueue::Add(SM_EVENT_CODE_TYPE eventCode, EventData eventData)
{
#ifdef DEBUG
    printf_P(PSTR("DEBUG: Adding event to queue\n"));
#endif
    uint8_t nextHead = (head + 1) % EVENT_QUEUE_LENGTH;

    if (nextHead != tail)
    {
        eventCodeBuffer[head] = eventCode;
        eventDataBuffer[head] = eventData;
        head = nextHead;
        return MC_STATUS_SUCCESS;
    }

    return MC_STATUS_EVENT_QUEUE_FULL;
}

// Remove from front of queue
void EventQueue::Remove(SM_EVENT_CODE_TYPE& eventCode, EventData& eventData)
{
#ifdef DEBUG
    printf_P(PSTR("DEBUG: Removing event from queue\n"));
#endif
    if (head != tail)
    {
        eventData = eventDataBuffer[tail];
        eventCode = eventCodeBuffer[tail];
        tail = (tail + 1) % EVENT_QUEUE_LENGTH;
    }
}

// Resets head and tail pointers
void EventQueue::Clear()
{
    head = 0;
    tail = 0;
}

// Query to determine if there are any elements in the queue
bool EventQueue::IsEmpty() const
{
    return head == tail;
}
