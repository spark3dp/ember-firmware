//  File: StateMachine.h
//  State machine declarations and include dependencies
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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#ifdef DEBUG
#include <avr/pgmspace.h>
#endif

// Must come before including smdefs header
typedef struct MotorControllerState MotorController_t;
#include "EventData.h"
#include "StateMachine_smdefs.h"
#include "MotorControllerState.h"

void MotorController_State_Machine_Reset_EventQueue();

#endif  // STATEMACHINE_H
