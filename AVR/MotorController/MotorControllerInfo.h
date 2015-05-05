/*
 * MotorControllerInfo.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef MOTORCONTROLLERINFO_H
#define MOTORCONTROLLERINFO_H

#include "AxisSettings.h"

/*
 * Type representing instantiation of state machine
 * All writable data referred to by state machine
 * related code goes in this structure
 */

struct MotorControllerInfo
{
    MotorController_state_t sm_state;
    AxisSettings zAxisSettings;
    AxisSettings rAxisSettings;
};

#endif /* MOTORCONTROLLERINFO_H */
