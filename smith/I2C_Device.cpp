/*
 * File:   I2C_Device.cpp
 * Author: Richard Greene
 * Implements an I2C device at a particular slave address
 * Created on March 13, 2014, 2:01 PM
 */

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <Hardware.h>
#include <I2C_Device.h>
#include <MessageStrings.h>
#include <Logger.h>

/// Public constructor, opens I2C connection and sets slave address
/// invalid slave address of 0xFF creates a null device that does nothing
/// except return '@' when reading
I2C_Device::I2C_Device(unsigned char slaveAddress)
{
    _isNullDevice = (slaveAddress == 0xFF);
    if(_isNullDevice)
        return;
    
    // open the I2C port
    char s[20];
    sprintf(s, "//dev//i2c-%d", I2C_PORT);
    _i2cFile = open(s, O_RDWR);
	if (_i2cFile < 0)
    {
		Logger::LogError(LOG_ERR, errno, I2C_FILE_OPEN_ERROR);
		exit(1);
	}

    // set the slave address for this device
    if (ioctl(_i2cFile, I2C_SLAVE, slaveAddress) < 0)
    {
        Logger::LogError(LOG_ERR, errno, I2C_SLAVE_ADDRESS_ERROR);
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
void I2C_Device::Write(unsigned char registerAddress, unsigned char data)
{
    if(_isNullDevice)
        return;
        
	_writeBuf[0] = registerAddress;
	_writeBuf[1] = data;

	if(write(_i2cFile, _writeBuf, 2) != 2) {
		Logger::LogError(LOG_WARNING, errno, I2C_WRITE_ERROR);
	}
}

/// Write a string of bytes to the given register
void I2C_Device::Write(unsigned char registerAddress, const unsigned char* data)
{
    if(_isNullDevice)
        return;
    
    int len = strlen((const char*)data);
    if(len > BUF_SIZE - 1) {
      Logger::LogError(LOG_WARNING, errno, I2C_LONG_STRING_ERROR);
      return;  
    }
	_writeBuf[0] = registerAddress;
    strncpy((char*)_writeBuf + 1, (const char*)data, len);
    len++;
	if(write(_i2cFile, _writeBuf, len) != len) {
		Logger::LogError(LOG_WARNING, errno, I2C_WRITE_ERROR);
        return;
	}
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
		Logger::LogError(LOG_ERR, errno, I2C_READ_WRITE_ERROR);
        return -1;
	}

	if(read(_i2cFile, _readBuf, 1) != 1){
		Logger::LogError(LOG_ERR, errno, I2C_READ_READ_ERROR);
        return -1;
	}
	
	return _readBuf[0];
}