/* 
 * File:   CommandPipe.cpp
 * Author: Jason Lefley
 *
 * Wrapper for named pipe used to accept commands from other processes
 *  
 * Created on August 11, 2015, 4:37 PM
 */

#include <sys/stat.h>
#include <stdlib.h> // remove if exit is not used
#include <fcntl.h>
#include <sys/epoll.h>

#include "CommandPipe.h"
#include "Filenames.h"
#include "Logger.h"

/*
 * Constructor, set up UICommand events originating from the command pipe as
 * level triggered (default) so data not read in a given iteration of the event
 * loop will still trigger epoll_wait() on the next iteration
 */
CommandPipe::CommandPipe() :
_events(EPOLLIN | EPOLLERR)
{
    //TODO: throw and handle exceptions if unable to create or open named pipe

    // Create the named pipe if it does not exist
    if (access(COMMAND_PIPE, F_OK) == -1)
    {
        if (mkfifo(COMMAND_PIPE, 0666) < 0)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(CommandPipeCreation));
            exit(-1);  // we can't really run if we can't accept commands
        }
    }

    // Open for both reading and writing
    // If we don't open for reading first, open will block
    _readFd = open(COMMAND_PIPE, O_RDONLY | O_NONBLOCK);
    _writeFd = open(COMMAND_PIPE, O_WRONLY | O_NONBLOCK);

    if (_readFd == -1)
    {
        std::cerr << "unable to open command pipe for reading" << std::endl;
        exit(-1);
    }

    if (_writeFd == -1)
    {
        std::cerr << "unable to open command pipe for writing" << std::endl;
        exit(-1);
    }
 
}

CommandPipe::~CommandPipe()
{
    remove(COMMAND_PIPE);
    close(_readFd);
    close(_writeFd);
}

uint32_t CommandPipe::GetEventTypes() const
{
    return _events;
}

int CommandPipe::GetFileDescriptor() const
{
    return _readFd;
}

/*
 * Read one new-line or null delimited message from the named pipe
 */
ResourceBufferVec CommandPipe::Read()
{
    char buf;
    ResourceBufferVec buffers;
    ResourceBuffer buffer;

    lseek(_readFd, 0, SEEK_SET);

    while (read(_readFd, &buf, 1) == 1)
    {
        if (buf == '\n' || buf == '\0')
            break;
        else
            buffer.push_back(buf);
    }
    
    buffers.push_back(buffer);

    return buffers;
}

bool CommandPipe::QualifyEvents(uint32_t events) const
{
    return _events & events;
}
