//  File: Planner.h
//  Motion planning functionality
//  For more information see plan_line.c and planner.h from TinyG
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from TinyG <https://www.synthetos.com/project/tinyg/>.
//
//  Copyright 2010 - 2015 Alden S. Hart Jr.
//  Copyright 2012 - 2015 Rob Giseburt
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

#ifndef PLANNER_H
#define PLANNER_H

#include "MotorController.h"
#include "Status.h"

namespace Planner
{
void Initialize(MotorController_t* mcState);
void SetAxisPosition(uint8_t axis, float position);
void SetPulsesPerUnit(uint8_t axis, float value);
Status PlanAccelerationLine(const float distances[], const uint8_t directions[], float speed, float maxSpeed);
Status PlanHoldCallback();
void BeginHold();
Status EndHold();
void EndMove();
}

#endif  // PLANNER_H
