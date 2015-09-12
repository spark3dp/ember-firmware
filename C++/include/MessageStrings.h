//  File:   MessageStrings.h
//  Defines text messages used by Ember
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

#ifndef MESSAGESTRINGS_H
#define	MESSAGESTRINGS_H

#define PRINTER_STARTUP_MSG         ("Autodesk Ember 3D Printer")
#define FW_VERSION_MSG              ("Firmware version: ")
#define BOARD_SER_NUM_MSG           ("Serial number: ")
#define NO_IP_ADDRESS               ("None")

#define PRINTER_STATUS_FORMAT       (", layer %d of %d, seconds left: %d")
#define LOG_STATUS_FORMAT           ("%s %s %s")
#define ERROR_FORMAT                "%s: %s"
#define LOG_ERROR_FORMAT            (ERROR_FORMAT "\n")
#define LOG_MOTOR_EVENT             ("motor interrupt: %d")
#define LOG_BUTTON_EVENT            ("button interrupt: %d")
#define LOG_DOOR_EVENT              ("door interrupt: %c")
#define LOG_KEYBOARD_INPUT          ("keyboard input: %s")
#define LOG_UI_COMMAND              ("UI command: %s")
#define LOG_TEMPERATURE_PRINTING ("printing layer #%d of %d: temperature = %g")
#define LOG_TEMPERATURE             ("temperature = %g")
#define LOG_JAM_DETECTED        ("jam detected at layer %d: temperature = %g")
#define LOG_NO_PROJECTOR_I2C        ("no I2C connection to projector")
#define LOG_INVALID_MOTOR_COMMAND   ("register: 0x%x, command: 0x%x")

#define UNKNOWN_REGISTRATION_CODE   ("unknown code")
#define UNKNOWN_REGISTRATION_URL    ("unknown URL")

#endif    // MESSAGESTRINGS_H

