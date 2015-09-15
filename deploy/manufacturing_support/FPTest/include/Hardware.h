//  File:   Hardware.h
//  Definitions of pins and other hardware-related items
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#ifndef HARDWARE_H
#define	HARDWARE_H

#define I2C2_PORT            (1) // corresponds to I2C2 on BBB, P9 pins 19 & 20
#define I2C1_PORT            (2) // corresponds to I2C1 on BBB, P9 pins 17 & 18

#define MOTOR_SLAVE_ADDRESS (0x10) // slave address for motor board
#define MOTOR_COMMAND       (0x20) // command register address for motor board
#define MOTOR_STATUS        (0x30) // status register address for motor board
#define MOTOR_INTERRUPT_PIN (60)   // GPIO 60 P9-12, interrupt from motor board

#define UI_SLAVE_ADDRESS    (0x11) // slave address for UI board
#define UI_COMMAND          (0x20) // command register address for UI board
// button status register for UI board, reading it clears button bits
#define BTN_STATUS          (0x30)  
#define DISPLAY_STATUS      (0x31) // display status register for UI board
#define UI_INTERRUPT_PIN    (30)   // GPIO 30 P9-11, interrupt from UI board

// status register values common to UI & motor board 
#define ERROR_STATUS        (0xFF) // there was an error on the board

// UI board status register values 
#define BTN1_PRESS          (0x01) // press of button 1 
#define BTN1_HOLD           (0x02) // hold of button 1 
#define BTN2_PRESS          (0x04) // press of button 2 
#define BTN2_HOLD           (0x08) // hold of button 2 
#define BTNS_1_AND_2_PRESS  (BTN1_PRESS | BTN2_PRESS)  // press of buttons 1 & 2
#define UI_BOARD_BUSY       (0x10) // indicates when commands cannot be sent

// UI board commands
#define CMD_START 0x98 // Frame start command
#define CMD_END 0x99 // Frame end command
#define CMD_SYNC 0x97 // Sync command
#define CMD_RESET 0x96 // Reset command
#define CMD_RING 0x01 // Put in ring command mode
#define CMD_OLED 0x02 // Put in OLED command mode
#define CMD_RING_OFF 0x01 // Turn ring off
#define CMD_RING_SEQUENCE 0x02 // Start a ring sequence (0 to stop)
#define CMD_RING_LED 0x03 // Set a ring LED to given value
#define CMD_RING_LEDS 0x04 // Set all ring LEDS to given value
#define CMD_OLED_SETTEXT 0x01 // Set OLED text flush left
#define CMD_OLED_CENTERTEXT 0x06 // Set OLED text centered on x
#define CMD_OLED_RIGHTTEXT 0x07 // Set OLED text flush right
#define CMD_OLED_CLEAR 0x02 // Clear OLED 
#define CMD_OLED_ON 0x03 // Turn OLED on
#define CMD_OLED_OFF 0x04 // Turn OLED off
// maximum string length for front panel'sI2C buffer
#define MAX_OLED_STRING_LEN (20) 
#define NUM_LEDS_IN_RING (21)

// motor board status register values 
#define SUCCESS             (0) // motor command completed successfully

#define ACK                 ('@')

// motor controller commands
#define HOME_COMMAND ('h')
#define MOVE_TO_START_POSN_COMMAND ('c')
#define FIRST_SEPARATE_COMMAND ('F')
#define BURNIN_SEPARATE_COMMAND ('B')
#define MODEL_SEPARATE_COMMAND ('P')
#define END_PRINT_COMMAND ('R')
#define LAYER_THICKNESS_COMMAND ("l%04d")
// FL = first layer, BI = burn-in layer, ML = model Layer
#define FL_SEPARATION_R_SPEED_COMMAND ("fa%06d")
#define FL_APPROACH_R_SPEED_COMMAND ("fb%06d")
#define FL_Z_LIFT_COMMAND ("fc%06d")
#define FL_SEPARATION_Z_SPEED_COMMAND ("fd%06d")
#define FL_APPROACH_Z_SPEED_COMMAND ("fe%06d")
#define FL_ROTATION_COMMAND ("ff%06d")
#define FL_EXPOSURE_WAIT_COMMAND ("fg%06d")
#define FL_SEPARATION_WAIT_COMMAND ("fh%06d")
#define FL_APPROACH_WAIT_COMMAND ("fi%06d")

#define BI_SEPARATION_R_SPEED_COMMAND ("ba%06d")
#define BI_APPROACH_R_SPEED_COMMAND ("bb%06d")
#define BI_Z_LIFT_COMMAND ("bc%06d")
#define BI_SEPARATION_Z_SPEED_COMMAND ("bd%06d")
#define BI_APPROACH_Z_SPEED_COMMAND ("be%06d")
#define BI_ROTATION_COMMAND ("bf%06d")
#define BI_EXPOSURE_WAIT_COMMAND ("bg%06d")
#define BI_SEPARATION_WAIT_COMMAND ("bh%06d")
#define BI_APPROACH_WAIT_COMMAND ("bi%06d")

#define ML_SEPARATION_R_SPEED_COMMAND ("pa%06d")
#define ML_APPROACH_R_SPEED_COMMAND ("pb%06d")
#define ML_Z_LIFT_COMMAND ("pc%06d")
#define ML_SEPARATION_Z_SPEED_COMMAND ("pd%06d")
#define ML_APPROACH_Z_SPEED_COMMAND ("pe%06d")
#define ML_ROTATION_COMMAND ("pf%06d")
#define ML_EXPOSURE_WAIT_COMMAND ("pg%06d")
#define ML_SEPARATION_WAIT_COMMAND ("ph%06d")
#define ML_APPROACH_WAIT_COMMAND ("pi%06d")

#define DOOR_SENSOR_PIN  (47)   // GPIO1_15
#define ROTATION_SENSOR_PIN  (27)   // GPIO0_27

#endif	/* HARDWARE_H */

