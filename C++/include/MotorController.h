//  File:   MotorController.h
//  Constants shared with motor controller firmware
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef MOTORCONTROLLER_H
#define	MOTORCONTROLLER_H

// High fencepost values need to equal one greater than the last code in
// the group.
// Low fencepost values need to equal one less then the first code in
// the group.

// NB: the AVR motor controller firmware relies on the first code in a group
// equaling 1 for groups other than the command register addresses.

// General commands are sent to the motor controller as a single byte
// without a register or parameter.
// Both smith and the AVR firmware use MC_GENERAL_REG to handle the construction
// and interpretation of general commands, but the register value itself is 
// never sent to the motor controller over I2C.

// Command register addresses, status register addresses, and general command 
// codes must not equal each other.

// command (write-only) register addresses 
#define MC_COMMAND_REG_LOW_FENCEPOST (0xA0)
#define MC_GENERAL_REG        (0xA1) // for general motor controller commands
#define MC_ROT_SETTINGS_REG   (0xA2) // for rotation settings  
#define MC_ROT_ACTION_REG     (0xA3) // for rotation actions 
#define MC_Z_SETTINGS_REG     (0xA4) // for Z axis settings  
#define MC_Z_ACTION_REG       (0xA5) // for Z axis actions
#define MC_COMMAND_REG_HIGH_FENCEPOST (0xA6)

// status (read-only) register address 
#define MC_STATUS_REG             (0x30) // gives motor controller status

// general motor controller commands (with no argument)
#define MC_GENERAL_LOW_FENCEPOST (0)
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
#define MC_ACTION_HIGH_FENCEPOST (3)

// status codes
// success, no errors
#define MC_STATUS_SUCCESS                              (0)  
// generic error return
#define MC_STATUS_ERROR                                (1) 
// function would block here (call again)
#define MC_STATUS_EAGAIN                               (2) 
// function had no-operation
#define MC_STATUS_NOOP                                 (3)
// operation is complete
#define MC_STATUS_COMPLETE                             (4)
// setting command unknown
#define MC_STATUS_SETTING_COMMAND_UNKNOWN              (5) 
// max jerk setting not in valid range
#define MC_STATUS_MAX_JERK_SETTING_INVALID             (6) 
// speed setting not in valid range
#define MC_STATUS_SPEED_SETTING_INVALID                (7) 
// microstepping mode not in valid range
#define MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID   (8) 
// units per revolution setting not in valid range
#define MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID (9) 
// step angle setting not in valid range
#define MC_STATUS_STEP_ANGLE_SETTING_INVALID           (10)
// no unused planner buffers available
#define MC_STATUS_PLANNER_BUFFER_FULL                  (11)
// command buffer capacity reached
#define MC_STATUS_COMMAND_BUFFER_FULL                  (12) 
// event queue capacity reached
#define MC_STATUS_EVENT_QUEUE_FULL                     (13) 
// unknown command received
#define MC_STATUS_COMMAND_UNKNOWN                      (14) 
// error handling state machine transition
#define MC_STATUS_STATE_MACHINE_ERROR                  (15) 
// move is less than minimum length
#define MC_STATUS_MOVE_LENGTH_TOO_SMALL                (16) 
// move is less than minimum time
#define MC_STATUS_MOVE_TIME_TOO_SMALL                  (17) 
// block is too short - was skipped
#define MC_STATUS_BLOCK_SKIPPED                        (18) 
// unrecoverable internal error
#define MC_STATUS_INTERNAL_ERROR                       (19) 

#endif    // MOTORCONTROLLER_H

