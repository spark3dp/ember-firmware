//  File:   I2C_Device.h
//  Defines an I2C device at a particular slave address
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

#ifndef I2C_DEVICE_H
#define	I2C_DEVICE_H

#include <Hardware.h>

#define BUF_SIZE 32

/// Defines an I2C device at a specific slave address with which the BBB
/// can communicate
class I2C_Device
{
public:
    I2C_Device(unsigned char slaveAddress, int port = I2C2_PORT);
    ~I2C_Device();
    bool Write(unsigned char registerAddress, unsigned char data);
    bool Write(unsigned char registerAddress, const unsigned char* data, 
               int len);
    unsigned char Read(unsigned char registerAddress);
    
protected:    
    // don't allow construction without specifying a slave address
    I2C_Device() {} 
    
private:
    int _i2cFile;    // file descriptor for this device
    unsigned char _writeBuf[BUF_SIZE];	// contains data to be written
	unsigned char _readBuf[BUF_SIZE];	// contains data that was read
    bool _isNullDevice;          // if true, creates a dummy device
};


#endif	/* I2C_DEVICE_H */

