//  File:   interface-commands.h
//  Interface commands shared between smith and front panel
//
//  This file is part of the Ember Front Panel firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Evan Davey  <http://www.ekidna.io/ember/>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * Commands are transmitted within a frame in the form:
 *    [CMD_START][FRAME LENGTH BYTE][PAYLOAD]
 *
 * For I2C transmissions, commands should be sent in 32 byte or less chunks
 * ## General Commands 
 * * [CMD_SYNC] - get interrupt from control panel
 * * [CMD_RESET] - software reset
 *
 * ## Ring Commands 
 * * [CMD_RING][CMD_RING_LED][LED][HI BYTE][LO BYTE] - set led (0 indexed) to given 16 bit value (0-4096)
 *
 * * [CMD_RING][CMD_RING_LEDS][HI BYTE][LO BYTE] - set all leds to given 16 bit value (0-4096)
 *
 * * [CMD_RING][CMD_RING_SEQUENCE][SEQUENCE NUMBER BYTE] - start the specified animation sequence (0 to 8)
 *
 * ## Oled Commands
 * * [CMD_OLED][CMD_OLED_ON] - turn on OLED display
 *
 * * [CMD_OLED][CMD_OLED_OFF] - turn off OLED display
 *
 * * [CMD_OLED][CMD_OLED_CLEAR] - clear OLED (to pre-defined background colour)
 *
 * * [CMD_OLED][CMD_OLED_TEXT][X BYTE][Y BYTE][SIZE BYTE][HI COLOR BYTE][LO
 * COLOR BYTE][TEXT LENGTH BYTE][TXT BYTES] - display text of specified length
 * in the specified color (16 bits) and size (1-2) and the specified x and y
 * position (0-127)
 *
 * * [CMD_OLED][CMD_OLED_CENTERTEXT][X BYTE][Y BYTE][SIZE BYTE][HI COLOR
 * BYTE][LO COLOR BYTE][TEXT LENGTH BYTE][TXT BYTES] - display text of
 * specified length in the specified color (16 bits) and size (1-2) and the
 * specified x and y position (0-127), centered around x 
 *
 */

#ifndef __INTERFACE_COMMANDS_H__
#define __INTERFACE_COMMANDS_H__

#define CMD_START 0x98 //!< Frame start command
#define CMD_END 0x99 //!< Frame start command
#define CMD_SYNC 0x97 //!< Sync command
#define CMD_RESET 0x96 //!< Reset command
#define CMD_RING 0x01 //!< Put in ring command mode
#define CMD_OLED 0x02 //!< Put in OLED command mode
#define CMD_SLEEP 0x03 //!< Set Sleep Mode Timing
#define CMD_RING_OFF 0x01 //!< Turn ring off
#define CMD_RING_SEQUENCE 0x02 //!< Start a ring sequence (0 to stop)
#define CMD_RING_LED 0x03 //!< Set a ring LED to given value
#define CMD_RING_LEDS 0x04 //!< Set all ring LEDS to given value
#define CMD_OLED_CENTERTEXT 0x06 //!< Set OLED text centered on x
#define CMD_OLED_RIGHTTEXT 0x07 //!< Set OLED text centered on x
#define CMD_OLED_TEXT 0x01 //!< Set OLED text 
#define CMD_OLED_CLEAR 0x02 //!< Clear OLED 
#define CMD_OLED_ON 0x03 //!< Turn OLED on
#define CMD_OLED_OFF 0x04 //!< Turn OLED off
#define CMD_OLED_PIXEL 0x05 //!< Set OLED pixel
#define CMD_OLED_LOGO 0x08 //!< Display logo

// Legacy
#define CMD_OLED_SETCENTEREDTEXT CMD_OLED_CENTERTEXT
#define CMD_OLED_SETTEXT CMD_OLED_TEXT
#define CMD_OLED_SETPIXEL CMD_OLED_PIXEL

// Registers

#define REG_BTN_STATUS       0x30
#define REG_DISPLAY_STATUS   0x31


#endif
