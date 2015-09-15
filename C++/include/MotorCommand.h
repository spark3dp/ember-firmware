//  File:   MotorCommands.h
//  I2C commands for controlling the motors
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

#ifndef MOTORCOMMANDS_H
#define	MOTORCOMMANDS_H

#include <sys/types.h>

#include <I2C_Device.h>

// A motor controller command that takes optional arguments.
class MotorCommand
{
public:
    MotorCommand(unsigned char cmdRegister, unsigned char cmd, 
                 int32_t value = 0);
    virtual bool Send(I2C_Device* i2c);

protected:  
    unsigned char _cmdRegister;
    unsigned char _cmd;
    int32_t _value;
};

#endif    // MOTORCOMMANDS_H

