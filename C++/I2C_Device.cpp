//  File:   I2C_Device.cpp
//  Implements an I2C device with which the Sitara can communicate
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#include "I2C_Device.h"

#include "Logger.h"
#include "Hardware.h"

// By specifying the stream buffer as a parameter, the I2C_Device can send data
// somewhere other than the actual I2C bus (i.e. for testing).
I2C_Device::I2C_Device(std::basic_streambuf<unsigned char>& streambuf) :
_stream(&streambuf)
{
}

I2C_Device::~I2C_Device()
{
}

// Write a single byte to the device
bool I2C_Device::Write(unsigned char data)
{
    _stream.clear();
    _stream << data;
    _stream.flush();
    
    if (_stream.fail()) 
    {
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cWrite));
        return false;
    }

    return true;
}

// Write a single byte to the given register
bool I2C_Device::Write(unsigned char registerAddress, unsigned char data)
{
    _stream.clear();
    _stream << registerAddress;
    _stream << data;
    _stream.flush();

    if (_stream.fail()) 
    {
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cWrite));
        return false;
    }

    return true;
}

// Write an array of bytes to the given register
bool I2C_Device::Write(unsigned char registerAddress, const unsigned char* data, 
                       int len)
{
    _stream.clear();
    _stream << registerAddress;

    for (int i = 0; i < len; i++)
    {
        _stream << data[i];
    }

    _stream.flush();

    if (_stream.fail()) 
    {
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cWrite));
        return false;
    }

    return true;
}

// Read a single byte from the given register
unsigned char I2C_Device::Read(unsigned char registerAddress)
{
    _stream.clear();
    _stream << registerAddress;
    _stream.flush();
    
    if (_stream.fail()) 
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cReadWrite));
        return ERROR_STATUS;
    }

    unsigned char buffer = _stream.get();

    if (_stream.fail()) 
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cReadRead));
        return ERROR_STATUS;
    }

    return buffer;
}