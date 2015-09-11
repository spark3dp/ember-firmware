//  File: Motors.h
//  Interface to stepper motors
//  Executes motion plan by dequeuing segments and generating appropriate step pulses
//  For more information see stepper.c and stepper.h from TinyG
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from TinyG <https://www.synthetos.com/project/tinyg/>.
//
//  Copyright 2010 - 2015 Alden S. Hart Jr.
//  Copyright 2013 - 2015 Robert Giseburt
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

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

#include "AxisSettings.h"
#include "StateMachine.h"

namespace Motors
{
void Initialize(MotorController_t* mc);
void SetMicrosteppingMode(uint8_t modeFlag);
void Disable();
void Enable();
Status SetNextSegment(float steps[], uint8_t directions[], float microseconds);
void SetNextSegmentNull();
void RequestMoveExecution();
}

#endif  // MOTORS_H
