//  File:   MotorCommands.cpp
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

#include <iostream>

#include <MotorCommand.h>
#include <MotorController.h>
#include <Logger.h>
#include <MessageStrings.h>

// Constructs a motor command that takes an optional 32-bit parameter
MotorCommand::MotorCommand(unsigned char cmdRegister, unsigned char cmd,
                           int32_t value) :
_cmdRegister(cmdRegister),
_cmd(cmd),
_value(value)
 {   
 }
 
// Sends a command to the motor controller, checking for valid commands and
// retrying in case there's an I2C write failure.
bool MotorCommand::Send(I2C_Device* i2c) 
{
    // don't allow zero values for settings and actions
    if (_cmdRegister != MC_GENERAL_REG && _value == 0)
    {
        char msg[100];
        sprintf(msg, LOG_INVALID_MOTOR_COMMAND, _cmdRegister, _cmd);
        LOGGER.HandleError(ZeroInMotorCommand, true, msg);
        return false;
    }
    
    // don't allow negative values for settings 
    if ((_cmdRegister == MC_ROT_SETTINGS_REG ||
        _cmdRegister == MC_Z_SETTINGS_REG) && _value < 0)
    {
        char msg[100];
        sprintf(msg, LOG_INVALID_MOTOR_COMMAND, _cmdRegister, _cmd);
        LOGGER.HandleError(NegativeInMotorCommand, true, msg);
        return false;
    }
    
    if (_cmdRegister == MC_GENERAL_REG)
    {
        // Communicate general commands using a single byte.
        // The firmware may send general commands, such as pause,
        // when the motor controller is executing a move and the possibility
        // of a write failure exists.
        // Although the writing of a single byte general command may fail, the
        // firmware can attempt the transmission again without having to worry
        // which of the 6 command bytes the motor controller received 
        // successfully.
        
        int tries = 0;
        while(tries++ < MAX_I2C_CMD_TRIES)
        {
            if (i2c->Write(_cmd))
                return true;  
        }
    }
    else
    {

//      std::cout << "Sending to register: " << std::hex << (int)_cmdRegister <<
//                   ", command " << (int)_cmd << 
//                   ", value " << std::dec << _value << std::hex <<
//                   " (" << (int)(_value & 0xFF) << ", " <<
//                           (int)((_value >> 8)  & 0xFF)  << ", " <<
//                           (int)((_value >> 16)  & 0xFF) << ", " <<
//                           (int)((_value >> 24)  & 0xFF) << ")"  <<  
//      std::endl; 

        unsigned char buf[5] = {_cmd, _value & 0xFF, 
                                     (_value >> 8)  & 0xFF,
                                     (_value >> 16) & 0xFF, 
                                     (_value >> 24) & 0xFF};
        
        int tries = 0;
        while(tries++ < MAX_I2C_CMD_TRIES)
        {
            if (i2c->Write(_cmdRegister, buf, 5))
                return true;   
        }
    }
    
    return false;
}