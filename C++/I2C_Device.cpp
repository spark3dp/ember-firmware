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

#include <sstream>
#include <fcntl.h>
#include <stdexcept>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "ErrorMessage.h"
#include "Logger.h"
#include "Hardware.h"

// Public constructor, opens I2C connection and sets slave address
I2C_Device::I2C_Device(unsigned char slaveAddress, int port)
{
    // open the I2C port
    std::ostringstream i2cFileNameStream;
    i2cFileNameStream << "/dev/i2c-" << port;
    std::string i2cFileName = i2cFileNameStream.str();
    
    _fd = open(i2cFileName.c_str(), O_RDWR);
    if (_fd < 0)
    {
        throw std::runtime_error(ErrorMessage::Format(I2cFileOpen, errno));
    }

    // set the slave address for this device
    if (ioctl(_fd, I2C_SLAVE, slaveAddress) < 0)
    {
        close(_fd);
        throw std::runtime_error(ErrorMessage::Format(I2cSlaveAddress, errno));
    }
}

// Closes connection to the device
I2C_Device::~I2C_Device()
{
    close(_fd);
}

// Write a single byte to the device
bool I2C_Device::Write(unsigned char data) const
{
    if (write(_fd, &data, 1) != 1) 
    {
        Logger::LogError(LOG_WARNING, errno, I2cWrite);
        return false;
    }

    return true;
}

// Write a single byte to the given register
bool I2C_Device::Write(unsigned char registerAddress, unsigned char data) const
{
    unsigned char buffer[2] = { registerAddress, data };

    if (write(_fd, &buffer, 2) != 2) 
    {
        Logger::LogError(LOG_WARNING, errno, I2cWrite);
        return false;
    }
    
    return true;
}

// Write an array of bytes to the given register
bool I2C_Device::Write(unsigned char registerAddress, const unsigned char* data, 
                       int length) const
{
    unsigned char buffer[length + 1];
    buffer[0] = registerAddress;
    memcpy(&buffer[1], data, length);

    if (write(_fd, &buffer[0], length + 1) != length + 1) 
    {
        Logger::LogError(LOG_WARNING, errno, I2cWrite);
        return false;
    }

    return true;
}

// Read a single byte from the given register
unsigned char I2C_Device::Read(unsigned char registerAddress) const
{
    if (write(_fd, &registerAddress, 1) != 1) 
    {
        Logger::LogError(LOG_ERR, errno, I2cReadWrite);
        return ERROR_STATUS;
    }

    unsigned char buffer;
    
    if (read(_fd, &buffer, 1) != 1)
    {
        Logger::LogError(LOG_ERR, errno, I2cReadRead);
        return ERROR_STATUS;
    }

    return buffer;
}

// Read an array of bytes from the given register
bool I2C_Device::Read(unsigned char registerAddress, unsigned char* data, 
                     int length) const
{
    if (write(_fd, &registerAddress, 1) != 1) 
    {
        Logger::LogError(LOG_ERR, errno, I2cReadWrite);
        return false;
    }

    unsigned char buffer[length];
    if (read(_fd, buffer, length) != length)
    {
        Logger::LogError(LOG_ERR, errno, I2cReadRead);
        return false;
    }
    
    memcpy(data, buffer, length);
    return true;
}

constexpr int MAX_READ_WHEN_READY_ATTEMPTS = 40;
constexpr unsigned int DELAY_5_Ms = 5000;

// Read a single byte from the given register, from a device (such as the 
// projector) that returns an initial byte indicating its readiness.
unsigned char I2C_Device::ReadWhenReady(unsigned char registerAddress, 
                                        unsigned char readyStatus) const
{
    if (write(_fd, &registerAddress, 1) != 1) 
    {
        Logger::LogError(LOG_ERR, errno, I2cReadWrite);
        return ERROR_STATUS;
    }

    unsigned char buffer[2];
    
    for(int i = 0; i < MAX_READ_WHEN_READY_ATTEMPTS; i++)
    {
        if (read(_fd, buffer, 2) != 2)
        {
            Logger::LogError(LOG_ERR, errno, I2cReadReadWhenReady);
            return ERROR_STATUS;
        }
        else if(buffer[0] == readyStatus)
            return buffer[1];
        else // wait a bit before next try
            usleep(DELAY_5_Ms);
            
    }
    // all attempts failed to find the device ready
    Logger::LogError(LOG_ERR, errno, I2cDeviceNotReady);
    return ERROR_STATUS;
}

bool I2C_Device::ReadWhenReady(unsigned char registerAddress, 
                               unsigned char* data, int length,
                               unsigned char readyStatus) const
{
    if (write(_fd, &registerAddress, 1) != 1) 
    {
        Logger::LogError(LOG_ERR, errno, I2cReadWrite);
        return false;
    }

    int lengthPlusOne = length + 1;
    unsigned char buffer[lengthPlusOne];
        
    for(int i = 0; i < MAX_READ_WHEN_READY_ATTEMPTS; i++)
    {
        
        if (read(_fd, buffer, lengthPlusOne) != lengthPlusOne)
        {
            Logger::LogError(LOG_ERR, errno, I2cReadReadWhenReady);
            return false;
        }
        else if(buffer[0] == readyStatus)
        {
            memcpy(data, &buffer[1], length);
            return true;
        }
    }
    // all attempts failed to find the device ready
    Logger::LogError(LOG_ERR, errno, I2cDeviceNotReady);
    return false; 
}
