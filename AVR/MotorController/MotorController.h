//  File: MotorController.h
//  Top level motor controller commands
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

#ifndef MOTORCONTROLLER_H_AVR
#define MOTORCONTROLLER_H_AVR

#include <stdint.h>

#include "AxisSettings.h"
#include "StateMachine.h"
#include "Status.h"

namespace MotorController
{
void Initialize(MotorController_t* mcState);
void GenerateInterrupt();
Status UpdateSettings(uint8_t axis, EventData eventData, AxisSettings& axisSettings);
Status HomeZAxis(int32_t homingDistance, MotorController_t* mcState);
Status HomeRAxis(int32_t homingDistance, MotorController_t* mcState);
Status Move(uint8_t motorIndex, int32_t distance, const AxisSettings& settings);
void EndMotion();
}

#endif  // MOTORCONTROLLER_H_AVR
