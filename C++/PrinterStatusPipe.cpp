/* 
 * File:   PrinterStatusQueue.cpp
 * Author: Jason Lefley
 *
 * Communicates printer status updates between different parts of program.
 *  
 * Created on August 12, 2015, 9:04 AM
 */

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdexcept>
#include <cerrno>

#include "PrinterStatusPipe.h"

/// Constructor
/// Create an eventfd instance for use as the signaling mechanism to the event
/// loop
PrinterStatusQueue::PrinterStatusQueue() :
_fd(eventfd(0, 0))
{
    if (_fd < 0)
        throw std::runtime_error(ErrorMessage::Format(EventfdCreate, errno));
}

PrinterStatusQueue::~PrinterStatusQueue()
{
    close(_fd);
}

uint32_t PrinterStatusQueue::GetEventTypes() const
{
    return EPOLLIN | EPOLLET;
}

int PrinterStatusQueue::GetFileDescriptor() const
{
    return _fd;
}

/// Return all status updates in the queue in FIFO order.
/// In a given event loop iteration, we want to propagate all status updates so
/// that the next iteration processes events against the actual state of the system.
/// Also read out and discard the number of "eventfd events" from the
/// eventfd instance.
EventDataVec PrinterStatusQueue::Read()
{
    EventDataVec eventData;

    uint64_t buffer;
    read(_fd, &buffer, sizeof(uint64_t));
    
    while (!_queue.empty())
    {
        eventData.push_back(EventData(_queue.front()));
        _queue.pop();
    }

    return eventData;
}

/// Add the specified printer status to the back of the queue and write to the
/// eventfd instance to signal availability of data.
void PrinterStatusQueue::Push(const PrinterStatus& printerStatus)
{
    _queue.push(printerStatus);
    uint64_t buffer = 1;
    write(_fd, &buffer, sizeof(uint64_t));
}

bool PrinterStatusQueue::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}
