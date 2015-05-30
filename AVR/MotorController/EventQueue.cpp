/*
 * EventQueue.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-09
 * Description: FIFO queue to hold events for deferred handling
 */


#include "EventQueue.h"
#include "Debug.h"

EventQueue::EventQueue()
{
}

EventQueue::~EventQueue()
{
}

/*
 * Add to end of queue
 */

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

/*
 * Remove from front of queue
 */

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

/*
 * Resets head and tail pointers
 */

void EventQueue::Clear()
{
    head = 0;
    tail = 0;
}

/*
 * Query to determine if there are any elements in the queue
 */

bool EventQueue::IsEmpty() const
{
    return head == tail;
}
