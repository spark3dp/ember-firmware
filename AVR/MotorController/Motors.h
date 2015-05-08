/*
 * Motors.h
 * Author: Jason Lefley
 * Date  : 2015-04-28
 */

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

#include "AxisSettings.h"
#include "StateMachine.h"

#define Z_AXIS_MOTOR 0
#define R_AXIS_MOTOR 1

namespace Motors
{
void Initialize(MotorController_t* mcState);
void Reset();
void SetMicrosteppingMode(uint8_t modeFlag);
void Enable();
void Disable();
}

#endif /* MOTORS_H */
