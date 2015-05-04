/* 
 * File:   MotorController.h
 * Author: Richard Greene
 *
 * Constants shared with motor controller firmware.
 * 
 * Created on April 28, 2015, 10:57 AM
 */

#ifndef MOTORCONTROLLER_H
#define	MOTORCONTROLLER_H

// the first value in each group needs to be 1
// high fencepost values need to be equal to one greater than the last
// code in the group

// command (write-only) register addresses 
#define MC_GENERAL_REG        (0x01) // for general motor controller commands 
#define MC_ROT_SETTINGS_REG   (0x02) // for rotation settings  
#define MC_ROT_ACTION_REG     (0x03) // for rotation actions 
#define MC_Z_SETTINGS_REG     (0x04) // for Z axis settings  
#define MC_Z_ACTION_REG       (0x05) // for Z axis actions

// status (read-only) register address 
#define MC_STATUS             (0x06) // gives motor controller status
#define MC_REG_HIGH_FENCEPOST (0x07)

// general motor controller commands (with no argument)
#define MC_INTERRUPT        (1) // generate an interrupt
#define MC_RESET            (2) // perform a software reset 
#define MC_CLEAR            (3) // clear all pending commands 
#define MC_PAUSE            (4) // pause the command in progress 
#define MC_RESUME           (5) // when paused, resume all pending command(s)  
#define MC_GENERAL_HIGH_FENCEPOST (6)

// settings commands, for either rotation or Z axis (with int argument, x)
#define MC_STEP_ANGLE       (1) // set number of degrees/1000 for each step
#define MC_UNITS_PER_REV    (2) // set number of units of travel
                                // (e.g. microns or degrees) 
                                // for each motor revolution
#define MC_MICROSTEPPING    (3) // set microstepping mode, 1 = full step,
                                // 2 = half step, ..., 6 = 1/32 step
#define MC_JERK             (4) // set maximum jerk for move command in 
                                // units/minute^3/1E6
#define MC_SPEED            (5) // set speed for move (units/minute)
#define MC_MAX_SPEED        (6) // set max speed overall (units/minute)
#define MC_SETTINGS_HIGH_FENCEPOST (7)


// action command, for either rotation or Z axis (with int argument, x)
// Note: Positive argument means clockwise rotation or upward Z motion.
#define MC_MOVE             (1) // move by specified amount (units)
#define MC_HOME             (2) // move to limit switch in positive direction
                                // (assuming that's reached before the number
                                // of units specified in the argument)
// action commands, for either rotation or Z axis (with no argument)
#define MC_ENABLE           (3) // enable the motor
#define MC_DISABLE          (4) // disable the motor 
#define MC_ACTION_HIGH_FENCEPOST (5)

// status codes
#define MC_SUCCESS          (0)
#define MC_ERROR            (0xFF)

#endif	/* MOTORCONTROLLER_H */

