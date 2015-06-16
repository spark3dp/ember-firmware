/*
 * Planner.h
 * Author: Jason Lefley
 * Date  : 2015-06-14
 */

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

#endif /* PLANNER_H */
