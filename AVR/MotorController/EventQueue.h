/*
 * EventQueue.h
 * Author: Jason Lefley
 * Date  : 2015-05-09
 */

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#define EVENT_QUEUE_LENGTH 16 // Capacity of queue in terms of how many events can be stored

#include <stdint.h>

#include "EventData.h"

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();

    void Add(SM_EVENT_CODE_TYPE eventCode, EventData eventData);
    void Remove(SM_EVENT_CODE_TYPE& eventCode, EventData& eventData);
    bool IsEmpty() const;

private:
    EventQueue(const EventQueue&);

private:
    uint8_t elementCount = 0;
    uint8_t head = 0;
    uint8_t tail = 0;
    EventData eventDataBuffer[EVENT_QUEUE_LENGTH];
    SM_EVENT_CODE_TYPE eventCodeBuffer[EVENT_QUEUE_LENGTH];

};

#endif /* EVENTQUEUE_H */
