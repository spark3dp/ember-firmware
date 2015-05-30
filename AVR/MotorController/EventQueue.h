/*
 * EventQueue.h
 * Author: Jason Lefley
 * Date  : 2015-05-09
 */

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

#endif /* EVENTQUEUE_H */
