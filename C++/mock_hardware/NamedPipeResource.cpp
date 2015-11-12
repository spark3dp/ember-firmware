//  File:   NamedPipeResource.cpp
//  Resource that reads data from a named pipe (for testing purposes)
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

#include "mock_hardware/NamedPipeResource.h"

#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <stdexcept>

NamedPipeResource::NamedPipeResource(const std::string& path, size_t dataSize) :
_dataSize(dataSize)
{
    _fd = open(path.c_str(), O_RDWR);

    if (_fd < 0)
    {
        throw std::runtime_error(
                "unable to open pipe for reading in NamedPipeResource");
    }
}

NamedPipeResource::~NamedPipeResource()
{
    close(_fd);
}

uint32_t NamedPipeResource::GetEventTypes() const
{
    return EPOLLIN;
}

int NamedPipeResource::GetFileDescriptor() const
{
    return _fd;
}

bool NamedPipeResource::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}

EventDataVec NamedPipeResource::Read()
{
    unsigned char buffer;
    EventDataVec eventData;

    if (read(_fd, &buffer, _dataSize) == _dataSize)
    {
        eventData.push_back(EventData(buffer));
    }

    return eventData;
}
