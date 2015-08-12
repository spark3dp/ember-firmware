/* 
 * File:   PrinterStatusPipe.cpp
 * Author: Jason Lefley
 * 
 * Created on August 12, 2015, 9:04 AM
 */

#include <sys/stat.h>
#include <stdlib.h> // remove if exit is not used
#include <fcntl.h>

#include "PrinterStatusPipe.h"
#include "PrinterStatus.h"
#include "Filenames.h"
#include "Logger.h"

PrinterStatusPipe::PrinterStatusPipe()
{
    // Create the named pipe if it does not exist
    if (access(PRINTER_STATUS_PIPE, F_OK) == -1)
    {
        if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(StatusPipeCreation));
            exit(-1);  // we can't really run if we can't accept commands
        }
    }

    // Open for both reading and writing
    // If we don't open for reading first, open will block
    _readFd = open(PRINTER_STATUS_PIPE, O_RDONLY | O_NONBLOCK);
    _writeFd = open(PRINTER_STATUS_PIPE, O_WRONLY | O_NONBLOCK);

    // Get the size of a printer status message
    _printerStatusSize = sizeof(PrinterStatus);
}

PrinterStatusPipe::~PrinterStatusPipe()
{
    if (access(PRINTER_STATUS_PIPE, F_OK) != -1)
        remove(PRINTER_STATUS_PIPE);

    if (_readFd != -1)
        close(_readFd);

    if (_writeFd != -1)
        close(_writeFd);
}

uint32_t PrinterStatusPipe::GetEventTypes()
{
    return EPOLLIN | EPOLLERR | EPOLLET;
}

int PrinterStatusPipe::GetFileDescriptor()
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
    
    while (read(_readFd, buf, _printerStatusSize) == _printerStatusSize)
    {
        ResourceBuffer buffer(buf, _printerStatusSize);
        buffers.push_back(buffer);
    }
    
    return buffers;
}
