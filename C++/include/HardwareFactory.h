//  File:   HardwareFactory.h
//  Factory functions for creating objects that interface with hardware
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

#ifndef HARDWAREFACTORY_H
#define	HARDWAREFACTORY_H

#include <memory>

#include "IFrameBuffer.h"

class IResource;
class I_I2C_Device;

typedef std::unique_ptr<I_I2C_Device> I2C_DevicePtr;
typedef std::unique_ptr<IResource> ResourcePtr;
typedef std::unique_ptr<IFrameBuffer> FrameBufferPtr;

namespace HardwareFactory
{
I2C_DevicePtr  CreateMotorControllerI2cDevice();
I2C_DevicePtr  CreateFrontPanelI2cDevice();
I2C_DevicePtr  CreateProjectorI2cDevice();
ResourcePtr    CreateMotorControllerInterruptResource();
ResourcePtr    CreateFrontPanelInterruptResource();
FrameBufferPtr CreateFrameBuffer(int width, int height);
};


#endif  // HARDWAREFACTORY_H
