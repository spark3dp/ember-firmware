/*
 * StateMachine.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

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

#endif /* STATEMACHINE_H */
