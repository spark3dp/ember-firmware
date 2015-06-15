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

#endif /* MOTORS_H */
