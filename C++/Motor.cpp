
/* 
 * File:   Motor.cpp
 * Author: Richard Greene
 * Implements a motor controlled via I2C
 * Created on March 13, 2014, 5:51 PM
 */

#include <Motor.h>
    
/// Public constructor, base class opens I2C connection and sets slave address
Motor::Motor(unsigned char slaveAddress) :
I2C_Device(slaveAddress)
{
 
}

/// Base class closes connection to the device
Motor::~Motor() 
{
    // TODO: disable motors
}    

/// Send a set of commands to the motor controller.  Returns false immediately 
/// if any of the commands cannot be sent.
bool Motor::SendCommands(std::vector<MotorCommand> commands)
{
    for(int i = 0; i < commands.size(); i++)
        if(!commands[i].Send(this))
            return false;
    
    return true;
}
