//  File:   Shared.h
//  Constants shared with system-level testing project (tests)
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
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

#ifndef MOCKHARDWARE_SHARED_H
#define MOCKHARDWARE_SHARED_H

constexpr const char* MOTOR_CONTROLLER_I2C_READ_PIPE = "motor_controller_i2c_read_pipe";
constexpr const char* MOTOR_CONTROLLER_I2C_WRITE_PIPE = "motor_controller_i2c_write_pipe";
constexpr const char* FRONT_PANEL_I2C_READ_PIPE = "front_panel_i2c_read_pipe";
constexpr const char* FRONT_PANEL_I2C_WRITE_PIPE = "front_panel_i2c_write_pipe";
constexpr const char* PROJECTOR_I2C_READ_PIPE = "projector_i2c_read_pipe";
constexpr const char* PROJECTOR_I2C_WRITE_PIPE = "projector_i2c_write_pipe";
constexpr const char* MOTOR_CONTROLLER_INTERRUPT_READ_PIPE = "motor_controller_interrupt_read_pipe";
constexpr const char* FRONT_PANEL_INTERRUPT_READ_PIPE = "front_panel_interrupt_read_pipe";
constexpr const char* FRAME_BUFFER_IMAGE = "frame_buffer_image.png";

#endif  // MOCKHARDWARE_SHARED_H
