//  File:   I2C_Device.h
//  Abstraction providing functionality to interact with I2C devices
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

#ifndef I2C_DEVICE_H
#define	I2C_DEVICE_H

#include "I_I2C_Device.h"

class I2C_Device : public I_I2C_Device
{
public:
    I2C_Device(unsigned char slaveAddress, int port);
    ~I2C_Device();
    bool Write(unsigned char data) const;
    bool Write(unsigned char registerAddress, unsigned char data) const;
    bool Write(unsigned char registerAddress, const unsigned char* data, 
               int length) const;
    unsigned char Read(unsigned char registerAddress) const;
    bool Read(unsigned char registerAddress, unsigned char* data, 
             int length) const;
    unsigned char ReadWhenReady(unsigned char registerAddress, 
                                unsigned char readyStatus) const;
    bool ReadWhenReady(unsigned char registerAddress, 
                      unsigned char* data, int length,
                      unsigned char readyStatus) const;
  
private:
    I2C_Device(const I2C_Device&);
    I2C_Device& operator=(const I2C_Device&);

    int _fd;
};


#endif    // I2C_DEVICE_H

