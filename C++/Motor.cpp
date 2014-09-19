
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

/// Send a single command to the motor controller.  
bool Motor::SendCommand(MotorCommand command)
{
    return command.Send(this);
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

/// Perform a software reset of the motor controller.
bool Motor::Reset()
{
    return(MotorCommand(MC_GENERAL_REG, MC_RESET).Send(this));
}

/// Enable (engage) both motors.  Return false if either can't be enabled.
bool Motor::EnableMotors()
{
    return(MotorCommand(MC_ROT_ACTION_REG, MC_ENABLE).Send(this) &&
           MotorCommand(MC_Z_ACTION_REG, MC_ENABLE).Send(this));
}

/// Disable (disengage) both motors.  Return false if either can't be disabled.
bool Motor::DisableMotors()
{
    return(MotorCommand(MC_ROT_ACTION_REG, MC_DISABLE).Send(this) &&
           MotorCommand(MC_Z_ACTION_REG, MC_DISABLE).Send(this));    
}

/// Pause the current motor command(s) in progress.
bool Motor::Pause()
{
    return(MotorCommand(MC_GENERAL_REG, MC_PAUSE).Send(this));
}

/// Resume the  motor command(s) pending at last pause.
bool Motor::Resume()
{
    return(MotorCommand(MC_GENERAL_REG, MC_RESUME).Send(this));
}

/// Clear pending motor command(s).  Typical use would be after a pause, to
/// implement a cancel.
bool Motor::ClearCommandQueue()
{
    return(MotorCommand(MC_GENERAL_REG, MC_CLEAR).Send(this));
}

