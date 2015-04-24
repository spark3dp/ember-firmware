
/*  * File:   MotorCommands.cpp
 * Author: Richard Greene
 *
 * I2C commands for controlling the motors.
 * 
 * Created on September 18, 2014, 9:30 AM
 */
#include <iostream>

#include <MotorCommands.h>

/// Constructs a motor command that takes an optional 32-bit parameter
MotorCommand::MotorCommand(unsigned char cmdRegister, unsigned char cmd,
                           int32_t value) :
 _cmdRegister(cmdRegister),
_cmd(cmd),
_value(value)
 {   
 }
 
 /// Sends a command to the motor controller
bool MotorCommand::Send(I2C_Device* i2c) 
{
#ifdef DEBUG
    std::cout << "Sending to register: " << (int)_cmdRegister <<
                 ", command " << (int)_cmd << 
                 ", value " << _value << 
                 " (" << (int)(_value & 0xFF) << ", " <<
                         (int)((_value >> 8)  & 0xFF)  << ", " <<
                         (int)((_value >> 16)  & 0xFF) << ", " <<
                         (int)((_value >> 24)  & 0xFF) << ")"  <<  std::endl;
#endif  

    unsigned char buf[5] = {_cmd, _value & 0xFF, 
                                 (_value >> 8)  & 0xFF,
                                 (_value >> 16) & 0xFF, 
                                 (_value >> 24) & 0xFF};
    
    return i2c->Write(_cmdRegister, buf, 5);
}