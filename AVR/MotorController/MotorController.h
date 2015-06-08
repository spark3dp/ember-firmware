/*
 * MotorController.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef MOTORCONTROLLER_H_AVR
#define MOTORCONTROLLER_H_AVR

#include <stdint.h>

#include "AxisSettings.h"
#include "StateMachine.h"
#include "Status.h"

// Structure to keep track of hold state
typedef struct cmSingleton
{
    uint8_t cycle_state;
    uint8_t motion_state;
    uint8_t hold_state;
} cmSingleton_t;

extern cmSingleton_t cm;

enum cmFeedholdState
{
    FEEDHOLD_OFF = 0, // no feedhold in effect
    FEEDHOLD_SYNC,    // start hold - sync to latest aline segment
    FEEDHOLD_PLAN,    // replan blocks for feedhold
    FEEDHOLD_DECEL,   // decelerate to hold point
    FEEDHOLD_HOLD,    // holding
    FEEDHOLD_END_HOLD // end hold (transient state to OFF)
};

enum cmCycleState
{
    CYCLE_OFF = 0,    // machine is idle
    CYCLE_MACHINING,  // machine in normal machining cycle
};

enum cmMotionState
{
    MOTION_STOP = 0,  // motion has stopped
    MOTION_RUN,       // machine is in motion
    MOTION_HOLD       // feedhold in progress
};

namespace MotorController
{
void Initialize(MotorController_t* mcState);
void GenerateInterrupt();
Status UpdateSettings(uint8_t axis, EventData eventData, AxisSettings& axisSettings);
Status HomeZAxis(int32_t homingDistance, MotorController_t* mcState);
Status HomeRAxis(int32_t homingDistance, MotorController_t* mcState);
void BeginMotionHold();
void EndMotionHold();
Status Move(uint8_t motorIndex, int32_t distance, const AxisSettings& settings);
void EndMotion();
}

#endif /* MOTORCONTROLLER_H_AVR */
