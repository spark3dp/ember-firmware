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

// status (read-only) register addresses 
#define MC_STATUS           (0x30) // gives motor controller status
#define MC_Z_POSITION       (0x31) // gives Z axis position, as % of max travel

// general motor controller commands (with no argument)
#define MC_INTERRUPT        (1) // generate an interrupt
#define MC_RESET            (2) // perform a software reset 
#define MC_CLEAR            (3) // clear all pending commands 
#define MC_PAUSE            (4) // pause the command in progress 
#define MC_RESUME           (5) // when paused, resume all pending command(s)  

// settings commands, for either rotation or Z axis (with int argument, x)
#define MC_STEP_ANGLE       (1) // set motor angle to x degrees/1000 
#define MC_GEAR_RATIO       (2) // set gear ratio to x 
#define MC_MICROSTEPPING    (3) // set microstepping to x 
#define MC_MAX_SPEED        (4) // set maximum speed to x RPM 
#define MC_START_SPEED      (5) // set starting speed to x RPM or mm/s
#define MC_ACCELERATION     (6) // set acceleration profile to x percent
#define MC_DECELERATION     (7) // set deceleration profile to x percent
#define MC_SPEED            (8) // set speed to x RPM or mm/s

// settings commands for Z axis only (with int argument, x)
#define MC_Z_SCREW_PITCH    (9) // set lead screw pitch to x microns 
#define MC_Z_MAX_TRAVEL     (10) // set maximum travel to x microns 

// action command, for either rotation or Z axis (with int argument, x)
// Note: argument of 0 means move until limit switch is tripped
#define MC_MOVE             (1) // move x degrees/1000 or x microns
// action commands, for either rotation or Z axis (with no argument)
#define MC_ENABLE           (2) // enable the motor
#define MC_DISABLE          (3) // disable the motor 


/// A motor controller command that doesn't have any arguments.
class MotorCommand
{
public:
    MotorCommand(unsigned char cmdRegister, unsigned char cmd);
    virtual bool Send(I2C_Device* i2c);

protected:
    // don't allow construction without specifying arguments
    MotorCommand() {} 
    
private:    
    unsigned char _cmdRegister;
    unsigned char _cmd;
};

/// A motor controller command that sets an integer value.
class MotorValueCommand : public MotorCommand
{
public:
    MotorValueCommand(unsigned char cmdRegister, unsigned char cmd, int value);
    virtual bool Send(I2C_Device* i2c);

protected:
    // don't allow construction without specifying arguments
    MotorValueCommand() {} 
    
private:    
    unsigned char _cmdRegister;
    unsigned char _cmd;
    int _value;
};

#endif	/* MOTORCOMMANDS_H */

