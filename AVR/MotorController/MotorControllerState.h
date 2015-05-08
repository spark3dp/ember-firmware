/*
 * MotorControllerState.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef MOTORCONTROLLERSTATE_H
#define MOTORCONTROLLERSTATE_H

#include "AxisSettings.h"

/*
 * Type representing instantiation of state machine
 * All writable data referred to by state machine
 * related code goes in this structure
 */

struct MotorControllerState
{
    MotorController_state_t sm_state;
    AxisSettings zAxisSettings;
    AxisSettings rAxisSettings;
    uint8_t volatile motionComplete;
};

#endif /* MOTORCONTROLLERSTATE_H */
