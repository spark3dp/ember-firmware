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
#define MESSAGESTRINGS_H

constexpr const char*  PRINTER_STARTUP_MSG       = "Autodesk Ember 3D Printer";
constexpr const char*  FW_VERSION_MSG            = "Firmware version: ";
constexpr const char*  BOARD_SER_NUM_MSG         = "Serial number: ";
constexpr const char*  UPDATING_DEFAULTS_MSG     = "updating defaults for following settings:";
constexpr const char*  NO_IP_ADDRESS             = "None";
constexpr const char*  DISABLING_GAMMA_MSG       = "Disabling projector's gamma correction";

constexpr const char*  PRINTER_STATUS_FORMAT     = ", layer %d of %d, seconds left: %d";
constexpr const char*  LOG_STATUS_FORMAT         = "%s %s %s";
constexpr const char*  LOG_EXPOSING_FORMAT       = "%s %s, layer %d of %d, seconds left: %d";
constexpr const char*  ERROR_FORMAT              = "%s: %s";
constexpr const char*  LOG_ERROR_FORMAT          = "%s: %s\n";
constexpr const char*  LOG_MOTOR_EVENT           = "motor interrupt: %d";
constexpr const char*  LOG_BUTTON_EVENT          = "button interrupt: %d";
constexpr const char*  LOG_DOOR_EVENT            = "door interrupt: %c";
constexpr const char*  LOG_KEYBOARD_INPUT        = "keyboard input: %s";
constexpr const char*  LOG_UI_COMMAND            = "UI command: %s";
constexpr const char*  LOG_TEMPERATURE_PRINTING  = "printing layer #%d of %d: temperature = %g";
constexpr const char*  LOG_TEMPERATURE           = "temperature = %g";
constexpr const char*  LOG_JAM_DETECTED          = "jam detected at layer %d: temperature = %g";
constexpr const char*  LOG_NO_PROJECTOR_I2C      = "no I2C connection to projector";
constexpr const char*  LOG_INVALID_MOTOR_COMMAND = "register: 0x%x, command: 0x%x";

constexpr const char*  UNKNOWN_REGISTRATION_CODE = "unknown code";
constexpr const char*  UNKNOWN_REGISTRATION_URL  = "unknown URL";

constexpr const char*  UPGRADE_PROJECTOR_BTN_LINE1  = "Upgrade";
constexpr const char*  UPGRADE_PROJECTOR_BTN_LINE2  = "projector";

#endif    // MESSAGESTRINGS_H

