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
#define MC_COMMAND_REG_HIGH_FENCEPOST (0x06)

// status (read-only) register address 
#define MC_STATUS_REG             (0x30) // gives motor controller status

// general motor controller commands (with no argument)
#define MC_INTERRUPT        (1) // generate an interrupt
#define MC_RESET            (2) // perform a software reset 
#define MC_CLEAR            (3) // clear all pending commands 
#define MC_PAUSE            (4) // pause the command in progress 
#define MC_RESUME           (5) // when paused, resume all pending command(s) 
#define MC_ENABLE           (6) // enable both motors
#define MC_DISABLE          (7) // disable both motors 
#define MC_GENERAL_HIGH_FENCEPOST (8)

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
#define MC_SETTINGS_HIGH_FENCEPOST (6)


// action command, for either rotation or Z axis (with int argument, x)
// Note: Positive argument means clockwise rotation or upward Z motion.
#define MC_MOVE             (1) // move by specified amount (units)
#define MC_HOME             (2) // move to limit switch in positive direction
                                // (assuming that's reached before the number
                                // of units specified in the argument)
// action commands, for either rotation or Z axis (with no argument)
#define MC_ACTION_HIGH_FENCEPOST (3)

// status codes
#define MC_STATUS_SUCCESS                              (0)
#define MC_STATUS_DISTANCE_INVALID                     (1)
#define MC_STATUS_MAX_JERK_SETTING_INVALID             (2)
#define MC_STATUS_SPEED_SETTING_INVALID                (3)
#define MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID   (4)
#define MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID (5)
#define MC_STATUS_STEP_ANGLE_SETTING_INVALID           (6)
#define MC_STATUS_SETTING_COMMAND_INVALID              (7)
#define MC_STATUS_PLANNER_BUFFER_FULL                  (8)
#define MC_STATUS_COMMAND_BUFFER_FULL                  (9)
#define MC_STATUS_EVENT_QUEUE_FULL                     (10)
#define MC_STATUS_COMMAND_UNKNOWN                      (11)

#endif	/* MOTORCONTROLLER_H */

