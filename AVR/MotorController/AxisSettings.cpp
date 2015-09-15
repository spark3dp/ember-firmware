//  File: AxisSettings.cpp
//  Encapsulates axis specific setting values and conversions
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

#include <math.h>

#include "AxisSettings.h"

AxisSettings::AxisSettings()
{
}

AxisSettings::~AxisSettings()
{
}

// Set the rotation angle per step of the motor driving this axis
// value The setting value in degrees/1000
Status AxisSettings::SetStepAngle(int32_t value)
{
    if (value <= 0) return MC_STATUS_STEP_ANGLE_SETTING_INVALID;

    stepAngle = static_cast<float>(value) / 1000;

    return MC_STATUS_SUCCESS;
}

// Set the number of units displaced per motor revolution of the motor driving this axis
// value The setting value in units
Status AxisSettings::SetUnitsPerRevolution(int32_t value)
{
    if (value <= 0) return MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID;

    unitsPerRevolution = static_cast<float>(value);
    
    return MC_STATUS_SUCCESS;
}

// Set the maximum allowable jerk during acceleration/deceleration
// value The setting value in units/minute^3/1e6
Status AxisSettings::SetMaxJerk(int32_t value)
{
    if (value <= 0) return MC_STATUS_MAX_JERK_SETTING_INVALID;

    maxJerk = static_cast<float>(value) * 1e6f;
    
    return MC_STATUS_SUCCESS;
}

// Set the target speed for movements 
// value The setting value in units/minute
Status AxisSettings::SetSpeed(int32_t value)
{
    if (value <= 0) return MC_STATUS_SPEED_SETTING_INVALID;

    speed = static_cast<float>(value);
    
    return MC_STATUS_SUCCESS;
}

// Set the microstepping mode for the motor driving this axis
// value A flag determining the microstepping mode to use
//          1 = full step, 2 = half step, ... 6 = 1/32 step
Status AxisSettings::SetMicrosteppingMode(uint8_t value)
{
    if (value == 0 || value > 6) return MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID;

    // Compute the number of motor steps per microsteps from the flag
    // microstepping factor = 2 ^ (mode flag - 1)
    microsteppingFactor = 1 << (value - 1);
    
    return MC_STATUS_SUCCESS;
}

// Return the number of pulses required to move this axis by one unit
float AxisSettings::PulsesPerUnit() const
{
    return (360 * microsteppingFactor) / (stepAngle * unitsPerRevolution);
}

// Return the maximum allowable jerk during acceleration for this axis in units/minute^3
float AxisSettings::MaxJerk() const
{
    return maxJerk;
}

// Return the current speed value
float AxisSettings::Speed() const
{
    return speed;
}

// Validate that the client of this settings object set the settings to valid values
// Return a status code indicating success or failure
Status AxisSettings::Validate() const
{
    if (maxJerk == AXIS_SETTINGS_DEFAULT_MAX_JERK)
        return MC_STATUS_MAX_JERK_SETTING_INVALID;

    if (speed == AXIS_SETTINGS_DEFAULT_SPEED)
        return MC_STATUS_SPEED_SETTING_INVALID;

    if (microsteppingFactor == AXIS_SETTINGS_DEFAULT_MICROSTEPPING_FACTOR)
        return MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID;

    if (unitsPerRevolution == AXIS_SETTINGS_DEFAULT_UNITS_PER_REVOLUTION)
        return MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID;

    if (stepAngle == AXIS_SETTINGS_DEFAULT_STEP_ANGLE)
        return MC_STATUS_STEP_ANGLE_SETTING_INVALID;

    return MC_STATUS_SUCCESS;
}
