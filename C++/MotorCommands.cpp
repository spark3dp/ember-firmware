
/*  * File:   MotorCommands.cpp
 * Author: Richard Greene
 *
 * I2C commands for controlling the motors.
 * 
 * Created on September 18, 2014, 9:30 AM
 */

#include <MotorCommands.h>

/// Constructs a motor command that doesn't have any arguments
MotorCommand::MotorCommand(unsigned char cmdRegister, unsigned char cmd) :
 _cmdRegister(cmdRegister),
_cmd(cmd)         
 {   
 }
 
 /// Sends a command with no arguments to the motor controller
bool MotorCommand::Send(I2C_Device* i2c)
{
    return i2c->Write(_cmdRegister, _cmd);
}

/// Constructs a motor command that sets an integer value
 MotorValueCommand::MotorValueCommand(unsigned char cmdRegister, 
                                      unsigned char cmd, int value) :
 MotorCommand(cmdRegister, cmd),
_value(value)         
 {   
 }
 
 /// Sends a command that sets an integer value to the motor controller
bool MotorValueCommand::Send(I2C_Device* i2c)
{
    unsigned char buf[5] = {_cmd, _value & 0xFF, 
                                 (_value >> 8)  & 0xFF,
                                 (_value >> 16) & 0xFF, 
                                 (_value >> 24) & 0xFF};
    
    return i2c->Write(_cmdRegister, buf, 5);
}