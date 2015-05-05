/*
 * Motors.h
 * Author: Jason Lefley
 * Date  : 2015-04-28
 */

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

#include "AxisSettings.h"

#define Z_AXIS_MOTOR 0
#define R_AXIS_MOTOR 1

namespace Motors
{
void Initialize();
void Reset();
void SetMicrosteppingMode(uint8_t modeFlag);
void Enable();
void Disable();
void Move(uint8_t motorIndex, int32_t distance, const AxisSettings& settings);
float GetMoveTimes(float* minTime, float integralDistance, float speed, float maxSpeed);
}

#endif /* MOTORS_H */
