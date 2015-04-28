/*  * File:   MotorCommands.h
 * Author: Richard Greene
 *
 * I2C commands for controlling the motors.
 * 
 * Created on September 18, 2014, 9:30 AM
 */

#ifndef MOTORCOMMANDS_H
#define	MOTORCOMMANDS_H

#include <sys/types.h>

#include <I2C_Device.h>

/// A motor controller command that takes optional arguments.
class MotorCommand
{
public:
    MotorCommand(unsigned char cmdRegister, unsigned char cmd, int32_t value = 0);
    virtual bool Send(I2C_Device* i2c);

protected:
    // don't allow construction without specifying arguments
    MotorCommand() {}    
    unsigned char _cmdRegister;
    unsigned char _cmd;
    int32_t _value;
};

#endif	/* MOTORCOMMANDS_H */

