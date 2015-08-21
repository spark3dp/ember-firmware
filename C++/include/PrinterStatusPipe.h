/* 
 * File:   PrinterStatusQueue.h
 * Author: Jason Lefley
 * 
 * Communicates printer status updates between different parts of program.
 *
 * Created on August 12, 2015, 9:04 AM
 */

#ifndef PRINTERSTATUSQUEUE_H
#define PRINTERSTATUSQUEUE_H

#include <queue>

#include "IResource.h"
#include "PrinterStatus.h"

class PrinterStatusQueue : public IResource
{
public:
    PrinterStatusQueue();
    ~PrinterStatusQueue();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    void Push(const PrinterStatus& printerStatus);
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    PrinterStatusQueue(const PrinterStatusQueue&);
    PrinterStatusQueue& operator=(const PrinterStatusQueue&);

private:
    int _fd;
    std::queue<PrinterStatus> _queue;
};

#endif /* PRINTERSTATUSQUEUE_H */

