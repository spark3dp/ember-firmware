//  File:   I2C_Resource.cpp
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

#include "I2C_Resource.h"
#include "I2C_Device.h"

I2C_Resource::I2C_Resource(IResource& resource, I2C_Device& i2cDevice,
        unsigned char readRegister) :
_resource(resource),
_i2cDevice(i2cDevice),
_readRegister(readRegister)
{
}

I2C_Resource::~I2C_Resource()
{
}

// Return the event types from the underlying timer resource
uint32_t I2C_Resource::GetEventTypes() const
{
    return _resource.GetEventTypes();
}

// Return the file descriptor from the underlying timer resource
int I2C_Resource::GetFileDescriptor() const
{
    return _resource.GetFileDescriptor();
}

// When a resource event occurs, read from both the underlying resource and the
// I2C device
// Return the data from the I2C device and discard the data from the underlying
// resource
EventDataVec I2C_Resource::Read()
{
    _resource.Read();

    EventDataVec eventData;
    eventData.push_back(EventData(_i2cDevice.Read(_readRegister)));
    return eventData;
}

bool I2C_Resource::QualifyEvents(uint32_t events) const
{
    return _resource.QualifyEvents(events);
}
