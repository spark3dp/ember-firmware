/*  * File:   MotorCommands.h
 * Author: Richard Greene
 *
 * I2C commands for controlling the motors.
 * 
 * Created on September 18, 2014, 9:30 AM
 */

#ifndef MOTORCOMMANDS_H
#define	MOTORCOMMANDS_H

#include <I2C_Device.h>

// command (write-only) register addresses 
#define MC_GENERAL_REG      (0x20) // for general motor controller commands 
#define MC_ROT_SETTINGS_REG (0x21) // for rotation settings  
#define MC_ROT_ACTION_REG   (0x22) // for rotation actions 
#define MC_Z_SETTINGS_REG   (0x23) // for Z axis settings  
#define MC_Z_ACTION_REG     (0x24) // for Z axis actions

// status (read-only) register address 
#define MC_STATUS           (0x30) // gives motor controller status

// general motor controller commands (with no argument)
#define MC_INTERRUPT        (1) // generate an interrupt
#define MC_RESET            (2) // perform a software reset 
#define MC_CLEAR            (3) // clear all pending commands 
#define MC_PAUSE            (4) // pause the command in progress 
#define MC_RESUME           (5) // when paused, resume all pending command(s)  

// settings commands, for either rotation or Z axis (with int argument, x)
#define MC_STEP_ANGLE       (1) // set number of degrees/1000 for each step
#define MC_UNITS_PER_REV    (2) // set number of units of travel
                                // (e.g. microns or degrees) 
                                // for each motor revolution
#define MC_MICROSTEPPING    (3) // set microstepping mode, 1 = full step,
                                // 2 = half step, ..., 6 = 1/32 step
#define MC_MAX_MOVE_JERK    (4) // set maximum jerk for move command in 
                                // units/minute^3/1E6
#define MC_MAX_HOME_JERK    (5) // set maximum jerk for homing, also 
                                // units/minute^3/1E6
#define MC_SPEED            (6) // set speed for move (units/minute)
#define MC_MAX_SPEED        (7) // set max speed overall (units/minute)


// action command, for either rotation or Z axis (with int argument, x)
// Note: Positive argument means clockwise rotation or upward Z motion.
#define MC_MOVE             (1) // move by specified amount (units)
#define MC_HOME             (2) // move to limit switch in positive direction
// action commands, for either rotation or Z axis (with no argument)
#define MC_ENABLE           (3) // enable the motor
#define MC_DISABLE          (4) // disable the motor 

// status codes
#define MC_SUCCESS          (0)
#define MC_ERROR            (0xFF)

/// A motor controller command that takes optional arguments.
class MotorCommand
{
public:
    MotorCommand(unsigned char cmdRegister, unsigned char cmd, int value = 0);
    virtual bool Send(I2C_Device* i2c);

protected:
    // don't allow construction without specifying arguments
    MotorCommand() {}    
    unsigned char _cmdRegister;
    unsigned char _cmd;
    int _value;
};

#endif	/* MOTORCOMMANDS_H */

