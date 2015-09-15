//  File:   I2C_Resource.h
//  Wrapper for a resource that, on event, reads data from an I2C device
//  rather than the underlying resource itself
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

#ifndef I2C_RESOURCE_H
#define	I2C_RESOURCE_H

#include "IResource.h"

class I2C_Device;

class I2C_Resource : public IResource
{
public:
    I2C_Resource(IResource& resource, I2C_Device& i2cDevice,
            unsigned char readRegister);
    ~I2C_Resource();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    bool QualifyEvents(uint32_t events) const;

private:
    // This class has reference members
    // Disable copy construction and copy assignment
    I2C_Resource(const I2C_Resource&);
    I2C_Resource& operator=(const I2C_Resource&);

private:
    IResource& _resource;
    I2C_Device& _i2cDevice;
    unsigned char _readRegister;
};

#endif    // I2C_RESOURCE_H

