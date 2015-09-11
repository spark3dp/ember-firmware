//  File:   CommandPipe.h
//  Wrapper for named pipe used to accept commands from other processes
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdexcept>
#include <cerrno>

#include "CommandPipe.h"
#include "Filenames.h"
#include "ErrorMessage.h"

CommandPipe::CommandPipe()
{
    // Create the named pipe if it does not exist
    if (access(COMMAND_PIPE, F_OK) < 0)
        if (mkfifo(COMMAND_PIPE, 0666) < 0)
            throw std::runtime_error(ErrorMessage::Format(CommandPipeCreation, 
                                     errno));

    // Open for both reading and writing
    // If we don't open for reading first, open will block
    _readFd = open(COMMAND_PIPE, O_RDONLY | O_NONBLOCK);

    if (_readFd < 0)
        throw std::runtime_error(ErrorMessage::Format(CommandPipeOpenForReading, 
                                 errno));

    _writeFd = open(COMMAND_PIPE, O_WRONLY | O_NONBLOCK);

    if (_writeFd < 0)
    {
        close(_readFd);
        throw std::runtime_error(ErrorMessage::Format(CommandPipeOpenForWriting, 
                                 errno));
    }
 
}

CommandPipe::~CommandPipe()
{
    remove(COMMAND_PIPE);
    close(_readFd);
    close(_writeFd);
}

// Set up UICommand events originating from the command pipe as
// level triggered (default) so data not read in a given iteration of the event
// loop will still trigger epoll_wait() on the next iteration
uint32_t CommandPipe::GetEventTypes() const
{
    return EPOLLIN | EPOLLERR;
}

int CommandPipe::GetFileDescriptor() const
{
    return _readFd;
}

// Read one new-line or null delimited message from the named pipe
EventDataVec CommandPipe::Read()
{
    char buffer;
    EventDataVec eventData;
    std::string command;

    lseek(_readFd, 0, SEEK_SET);

    while (read(_readFd, &buffer, 1) == 1)
    {
        if (buffer == '\n' || buffer == '\0')
            break;
        else
            command.push_back(buffer);
    }
    
    eventData.push_back(EventData(command));

    return eventData;
}

bool CommandPipe::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}
