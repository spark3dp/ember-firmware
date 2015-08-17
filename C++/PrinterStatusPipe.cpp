/* 
 * File:   PrinterStatusPipe.cpp
 * Author: Jason Lefley
 *
 * Communicates printer status updates between different parts of program
 *  
 * Created on August 12, 2015, 9:04 AM
 */

// TODO: consider using eventfd() since printer status communication remains solely in this process

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>

#include "PrinterStatusPipe.h"
#include "PrinterStatus.h"
#include "Filenames.h"

PrinterStatusPipe::PrinterStatusPipe() :
_printerStatusSize(sizeof(PrinterStatus)),
_events(EPOLLIN | EPOLLERR | EPOLLET)
{
    // Create the named pipe if it does not exist
    if (access(PRINTER_STATUS_PIPE, F_OK) < 0)
        if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0)
            throw std::runtime_error(ErrorMessage::Format(StatusPipeCreation, errno));

    // Open for both reading and writing
    // If we don't open for reading first, open will block
    _readFd = open(PRINTER_STATUS_PIPE, O_RDONLY | O_NONBLOCK);

    if (_readFd < 0)
        throw std::runtime_error("unable to open printer status pipe for reading");

    _writeFd = open(PRINTER_STATUS_PIPE, O_WRONLY | O_NONBLOCK);
    
    if (_writeFd < 0)
    {
        close(_readFd);
        throw std::runtime_error("unable to open printer status pipe for writing");
    }
}

PrinterStatusPipe::~PrinterStatusPipe()
{
    remove(PRINTER_STATUS_PIPE);
    close(_readFd);
    close(_writeFd);
}

uint32_t PrinterStatusPipe::GetEventTypes() const
{
    return _events;
}

int PrinterStatusPipe::GetFileDescriptor() const
{
    return _readFd;
}

/*
 * Read all available data
 * In a given event loop iteration, we want to propagate all status updates so that the next iteration
 * processes events against the actual state of the system
 */
ResourceBufferVec PrinterStatusPipe::Read()
{
    ResourceBufferVec buffers;
    char buf[_printerStatusSize];
   
    lseek(_readFd, 0, SEEK_SET);
    
    while (read(_readFd, buf, _printerStatusSize) == _printerStatusSize)
    {
        ResourceBuffer buffer(buf, _printerStatusSize);
        buffers.push_back(buffer);
    }
    
    return buffers;
}

/*
 * Write the specified printer status data to the printer status pipe
 */
void PrinterStatusPipe::WriteStatus(PrinterStatus* pPrinterStatus)
{
    write(_writeFd, pPrinterStatus, _printerStatusSize);
}

bool PrinterStatusPipe::QualifyEvents(uint32_t events) const
{
    return _events & events;
}
