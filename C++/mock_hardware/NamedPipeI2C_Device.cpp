//  File:   NamedPipeI2C_Device.cpp
//  I2C device that reads and writes from named pipes (for testing purposes)
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

#include "mock_hardware/NamedPipeI2C_Device.h"

#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>

#include "Hardware.h"

NamedPipeI2C_Device::NamedPipeI2C_Device(const std::string& readPipePath,
                                         const std::string& writePipePath)
{
    _readFd = open(readPipePath.c_str(), O_RDWR);

    if (_readFd < 0)
    {
        throw std::runtime_error(
                "unable to open pipe for reading in NamedPipeI2C_Device");
    }
    
    _writeFd = open(writePipePath.c_str(), O_WRONLY | O_NONBLOCK);

    if (_writeFd < 0)
    {
        close(_readFd);
        throw std::runtime_error(
                "unable to open pipe for writing in NamedPipeI2C_Device");
    }
}

NamedPipeI2C_Device::~NamedPipeI2C_Device()
{
    close(_writeFd);
    close(_readFd);
}

bool NamedPipeI2C_Device::Write(unsigned char data) const
{
    if (write(_writeFd, &data, 1) != 1)
    {
        return false;
    }

    return true;
}

bool NamedPipeI2C_Device::Write(unsigned char registerAddress, unsigned char data) const
{
    unsigned char buffer[2] = { registerAddress, data };

    if (write(_writeFd, &buffer, 2) != 2) 
    {
        return false;
    }
    
    return true;
}

bool NamedPipeI2C_Device::Write(unsigned char registerAddress,
                                const unsigned char* data, int length) const
{
    unsigned char buffer[length + 1];
    buffer[0] = registerAddress;
    memcpy(&buffer[1], data, length);

    if (write(_writeFd, &buffer[0], length + 1) != length + 1) 
    {
        return false;
    }

    return true;
}

unsigned char NamedPipeI2C_Device::Read(unsigned char registerAddress) const
{
    if (write(_writeFd, &registerAddress, 1) != 1) 
    {
        return ERROR_STATUS;
    }

    unsigned char buffer;
    
    if (read(_readFd, &buffer, 1) != 1)
    {
        return ERROR_STATUS;
    }

    return buffer;
}
