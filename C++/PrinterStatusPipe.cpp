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
#include <stdlib.h> // remove if exit is not used
#include <fcntl.h>

#include "PrinterStatusPipe.h"
#include "PrinterStatus.h"
#include "Filenames.h"
#include "Logger.h"

PrinterStatusPipe::PrinterStatusPipe() :
_printerStatusSize(sizeof(PrinterStatus)),
_events(EPOLLIN | EPOLLERR | EPOLLET)
{
    //TODO: remove calls to exit() and throw and handle exceptions if unable to create or open named pipe
    
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

    if (_readFd == -1)
    {
        std::cerr << "unable to open printer status pipe for reading" << std::endl;
        exit(-1);
    }

    if (_writeFd == -1)
    {
        std::cerr << "unable to open printer status pipe for writing" << std::endl;
        exit(-1);
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
