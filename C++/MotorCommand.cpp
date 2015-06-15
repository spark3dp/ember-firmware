
/*  * File:   MotorCommands.cpp
 * Author: Richard Greene
 *
 * I2C commands for controlling the motors.
 * 
 * Created on September 18, 2014, 9:30 AM
 */
#include <iostream>

#include <MotorCommand.h>
#include <MotorController.h>
#include <Logger.h>
#include <MessageStrings.h>

/// Constructs a motor command that takes an optional 32-bit parameter
MotorCommand::MotorCommand(unsigned char cmdRegister, unsigned char cmd,
                           int32_t value) :
_cmdRegister(cmdRegister),
_cmd(cmd),
_value(value)
 {   
 }
 
/// Sends a command to the motor controller, checking for valid commands and
/// retrying in case there's an I2C write failure.
bool MotorCommand::Send(I2C_Device* i2c) 
{
    // don't allow zero values for settings and actions
    if(_cmdRegister != MC_GENERAL_REG && _value == 0)
    {
        char msg[100];
        sprintf(msg, LOG_INVALID_MOTOR_COMMAND, _cmdRegister, _cmd);
        LOGGER.HandleError(ZeroInMotorCommand, true, msg);
        return false;
    }
    
    // don't allow negative values for settings 
    if((_cmdRegister == MC_ROT_SETTINGS_REG ||
        _cmdRegister == MC_Z_SETTINGS_REG) && _value < 0)
    {
        char msg[100];
        sprintf(msg, LOG_INVALID_MOTOR_COMMAND, _cmdRegister, _cmd);
        LOGGER.HandleError(NegativeInMotorCommand, true, msg);
        return false;
    }
    
    if(_cmdRegister == MC_GENERAL_REG)
    {
        // communicate general commands using single byte
        // the firmware may send general commands, such as pause,
        // when the motor controller is executing a move and the possibility
        // of a write failure exists
        // although the writing of a single byte general command may fail, the
        // firmware can attempt the transmission again without having to worry
        // which of the 6 command bytes the motor controller received successfully

#ifdef DEBUG
        std::cout << "Sending general command: " << std::hex << (int)_cmd << 
                std::endl;
#endif
        
        int tries = 0;
        while(tries++ < MAX_I2C_CMD_TRIES)
        {
            if(i2c->Write(_cmd))
                return true;
            
#ifdef DEBUG
            std::cout << "Tried to send motor command " << tries 
                      << " times" << std::endl; 
#endif   
        }
    }
    else
    {
#ifdef DEBUG
        std::cout << "Sending to register: " << std::hex << (int)_cmdRegister <<
                     ", command " << (int)_cmd << 
                     ", value " << std::dec << _value << std::hex <<
                     " (" << (int)(_value & 0xFF) << ", " <<
                             (int)((_value >> 8)  & 0xFF)  << ", " <<
                             (int)((_value >> 16)  & 0xFF) << ", " <<
                             (int)((_value >> 24)  & 0xFF) << ")"  <<  std::endl;
#endif  

        unsigned char buf[5] = {_cmd, _value & 0xFF, 
                                     (_value >> 8)  & 0xFF,
                                     (_value >> 16) & 0xFF, 
                                     (_value >> 24) & 0xFF};
        
        int tries = 0;
        while(tries++ < MAX_I2C_CMD_TRIES)
        {
            if(i2c->Write(_cmdRegister, buf, 5))
                return true;
            
#ifdef DEBUG
            std::cout << "Tried to send motor command " << tries 
                      << " times" << std::endl; 
#endif   
        }
    }
    
    return false;
}