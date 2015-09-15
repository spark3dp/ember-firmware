//  File:   I2C_Device.cpp
//  Implements an I2C device at a particular slave address
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <Hardware.h>
#include <I2C_Device.h>
//#include <Logger.h>
//#include <ErrorMessage.h>

/// Public constructor, opens I2C connection and sets slave address
/// invalid slave address of 0xFF creates a null device that does nothing
/// except return '@' when reading
I2C_Device::I2C_Device(unsigned char slaveAddress, int port)
{
    _isNullDevice = (slaveAddress == 0xFF);
    if(_isNullDevice)
        return;
    
    // open the I2C port
    char s[20];
    sprintf(s, "//dev//i2c-%d", port);
    _i2cFile = open(s, O_RDWR);
	if (_i2cFile < 0)
    {
//		LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cFileOpen));
		exit(1);
	}

    // set the slave address for this device
    if (ioctl(_i2cFile, I2C_SLAVE, slaveAddress) < 0)
    {
//        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cSlaveAddress));
        exit(1);
	}
}

/// Closes connection to the device
I2C_Device::~I2C_Device()
{
    if(_isNullDevice)
        return;
    
    close(_i2cFile);
}

/// Write a single byte to the given register
bool I2C_Device::Write(unsigned char registerAddress, unsigned char data)
{
    if(_isNullDevice)
        return true;
        
	_writeBuf[0] = registerAddress;
	_writeBuf[1] = data;

	if(write(_i2cFile, _writeBuf, 2) != 2) {
//		LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cWrite));
        return false;
	}
    return true;
}

/// Write an array of bytes to the given register
bool I2C_Device::Write(unsigned char registerAddress, const unsigned char* data, 
                       int len)
{
    if(_isNullDevice)
        return true;
    
    if(len > BUF_SIZE - 1) {
//      LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cLongString));
      return false;  
    }
	_writeBuf[0] = registerAddress;
    memcpy((char*)_writeBuf + 1, (const char*)data, len);
    len++;
	if(write(_i2cFile, _writeBuf, len) != len) {
//		LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(I2cWrite));
        return false;
	}
    return true;
}

/// Read a single byte from the given register
unsigned char I2C_Device::Read(unsigned char registerAddress)
{
    if(_isNullDevice)
        return ACK;
    
#ifdef DEBUG  
 //   printf("\nabout to read I2C device\n");
#endif    
    
	_writeBuf[0] = registerAddress;
	
	if(write(_i2cFile, _writeBuf, 1) != 1) {
//		LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cReadWrite));
        return -1;
	}

	if(read(_i2cFile, _readBuf, 1) != 1){
//		LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cReadRead));
        return -1;
	}
	
	return _readBuf[0];
}