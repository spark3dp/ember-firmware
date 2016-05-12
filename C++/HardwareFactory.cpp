//  File:   HardwareFactory.cpp
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

#include "HardwareFactory.h"

#include "Hardware.h"
#include "I2C_Device.h"
#include "GPIO_Interrupt.h"
#include "Settings.h"
#include "FrameBuffer.h"

I2C_DevicePtr HardwareFactory::CreateMotorControllerI2cDevice()
{
    return I2C_DevicePtr(new I2C_Device(MOTOR_SLAVE_ADDRESS, I2C2_PORT));
}

I2C_DevicePtr HardwareFactory::CreateFrontPanelI2cDevice()
{
    return I2C_DevicePtr(new I2C_Device(FP_SLAVE_ADDRESS,
            PrinterSettings::Instance().GetInt(HARDWARE_REV) == 0 ? 
                                                        I2C2_PORT : I2C1_PORT));
}

//StreamBufferPtr HardwareFactory::CreateProjectorI2cDevice()
//{
//}

ResourcePtr HardwareFactory::CreateMotorControllerInterruptResource()
{
    return ResourcePtr(new GPIO_Interrupt(MOTOR_INTERRUPT_PIN,
            GPIO_INTERRUPT_EDGE_RISING));
}

ResourcePtr HardwareFactory::CreateFrontPanelInterruptResource()
{
    return ResourcePtr(new GPIO_Interrupt(FP_INTERRUPT_PIN,
            GPIO_INTERRUPT_EDGE_RISING));
}

FrameBufferPtr HardwareFactory::CreateFrameBuffer(int width, int height)
{
    return FrameBufferPtr(new FrameBuffer(width, height));
}