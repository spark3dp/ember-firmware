//  File:   HardwareFactory.cpp
//  Factory functions for creating objects having ability to mock actual
//  hardware (for testing purposes)
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

#include "HardwareFactory.h"

#include "mock_hardware/Shared.h"
#include "mock_hardware/NamedPipeResource.h"
#include "mock_hardware/NamedPipeI2C_Device.h"
#include "mock_hardware/ImageWritingFrameBuffer.h"

I2C_DevicePtr HardwareFactory::CreateMotorControllerI2cDevice()
{
    return I2C_DevicePtr(new NamedPipeI2C_Device(
            MOTOR_CONTROLLER_I2C_READ_PIPE, MOTOR_CONTROLLER_I2C_WRITE_PIPE));
}

I2C_DevicePtr HardwareFactory::CreateFrontPanelI2cDevice()
{
    return I2C_DevicePtr(new NamedPipeI2C_Device(
            FRONT_PANEL_I2C_READ_PIPE, FRONT_PANEL_I2C_WRITE_PIPE));
}

//I2C_DevicePtr HardwareFactory::CreateProjectorI2cDevice()
//{
//    return I2C_DevicePtr(new NamedPipeI2C_Device(
//            PROJECTOR_I2C_READ_PIPE, PROJECTOR_I2C_WRITE_PIPE));
//}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new NamedPipeResource(
            MOTOR_CONTROLLER_INTERRUPT_READ_PIPE, 1));
}

ResourcePtr HardwareFactory::CreateFrontPanelInterruptResource() 
{
    return ResourcePtr(new NamedPipeResource(
            FRONT_PANEL_INTERRUPT_READ_PIPE, 1));
}

FrameBufferPtr HardwareFactory::CreateFrameBuffer(int width, int height)
{
    return FrameBufferPtr(new ImageWritingFrameBuffer(width, height,
            FRAME_BUFFER_IMAGE));
}