//  File:   NullI2C_Device.hpp
//  I2C device implementation with no-op methods (for testing purposes)
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

#ifndef NULLI2C_DEVICE_HPP
#define	NULLI2C_DEVICE_HPP

#include "I_I2C_Device.h"

class NullI2C_Device : public I_I2C_Device
{
public:
    NullI2C_Device() {}
    ~NullI2C_Device() {}

    bool Write(unsigned char data) const { return true; }
    bool Write(unsigned char registerAddress, unsigned char data) const
        { return true; }
    bool Write(unsigned char registerAddress, const unsigned char* data, 
               int length) const { return true; }
    unsigned char Read(unsigned char registerAddress) const { return 0x00; }
    bool Read(unsigned char registerAddress, unsigned char* data, 
             int length) const { return true; };
    unsigned char ReadWhenReady(unsigned char registerAddress,
           unsigned char readyStatus) const { return 0x01; }
    bool ReadWhenReady(unsigned char registerAddress, 
                       unsigned char* data, int length,
                       unsigned char readyStatus) const { return true; }
private:
    NullI2C_Device(const NullI2C_Device&);
    NullI2C_Device& operator=(const NullI2C_Device&);
};

#endif	/* NULLI2C_DEVICE_HPP */

