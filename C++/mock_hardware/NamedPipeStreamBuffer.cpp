//  File:   NamedPipeStreamBuffer.cpp
//  Stream buffer implementation backed by a pair of named pipes
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

#include "mock_hardware/NamedPipeStreamBuffer.h"

#include <iostream>
#include <fcntl.h>
#include <stdexcept>

NamedPipeStreamBuffer::NamedPipeStreamBuffer(const std::string& readPipePath,
                                             const std::string& writePipePath) :
_lastRead(traits_type::eof())
{
    _readFd = open(readPipePath.c_str(), O_RDWR);

    if (_readFd < 0)
    {
        throw std::runtime_error(
                "unable to open pipe for reading in NamedPipeStreamBuffer");
    }
    
    _writeFd = open(writePipePath.c_str(), O_WRONLY | O_NONBLOCK);

    if (_writeFd < 0)
    {
        close(_readFd);
        throw std::runtime_error(
                "unable to open pipe for writing in NamedPipeStreamBuffer");
    }
}

NamedPipeStreamBuffer::~NamedPipeStreamBuffer()
{
    close(_writeFd);
    close(_readFd);
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::overflow(int_type ch)
{
    if (write(_writeFd, &ch, 1) == 1)
    {
        return ch;
    }

    return traits_type::eof();
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::underflow()
{
    return _lastRead;
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::uflow()
{
    unsigned char buffer;
    if (read(_readFd, &buffer, 1) == 1)
    {
        _lastRead = traits_type::to_int_type(buffer);
    }
    else
    {
        _lastRead = traits_type::eof();
    }
    return _lastRead;
}
