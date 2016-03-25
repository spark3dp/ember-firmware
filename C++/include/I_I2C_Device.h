//  File:   I_I2C_Device.h
//  Interface specification for I2C based communication
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

#ifndef I_I2C_DEVICE_H
#define	I_I2C_DEVICE_H

constexpr int MAX_I2C_CMD_TRIES = 2;   

class I_I2C_Device
{
public:
    virtual ~I_I2C_Device() {}
    virtual bool Write(unsigned char data) const = 0;
    virtual bool Write(unsigned char registerAddress, unsigned char data) const = 0;
    virtual bool Write(unsigned char registerAddress, const unsigned char* data,
                       int length) const = 0;
    virtual unsigned char Read(unsigned char registerAddress) const = 0;
    virtual bool Read(unsigned char registerAddress, unsigned char* data, 
                      int length) const = 0;
    virtual unsigned char ReadWhenReady(unsigned char registerAddress, 
                                        unsigned char readyStatus) const = 0;
    virtual bool ReadWhenReady(unsigned char registerAddress, 
                               unsigned char* data, int length,
                               unsigned char readyStatus) const = 0;
};

#endif  // I_I2C_DEVICE_H

