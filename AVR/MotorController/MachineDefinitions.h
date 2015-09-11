//  File: MachineDefinitions.h
//  Machine specific definitions
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MACHINEDEFINITIONS_H
#define MACHINEDEFINITIONS_H

#define Z_AXIS 0
#define R_AXIS 1
#define AXES_COUNT 2

#define FORWARD_DIRECTION 0
#define REVERSE_DIRECTION 1

// These flags allow distances with a given sign to move
// the motors in a particular direction
// Set to 0 or 1
#define Z_AXIS_MOTOR_POLARITY 0
#define R_AXIS_MOTOR_POLARITY 0

#endif  // MACHINEDEFINITIONS_H
