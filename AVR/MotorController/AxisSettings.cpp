/*
 * AxisSettings.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-28
 * Description: <describe this file>
 */


#include "AxisSettings.h"


AxisSettings::AxisSettings()
{
}

AxisSettings::~AxisSettings()
{
}

/*
 * Set the rotation angle per step of the motor driving this axis
 * value The setting value in degrees/1000
 */

void AxisSettings::SetStepAngle(int32_t value)
{
    stepAngle = static_cast<float>(value) / 1000;
    ComputePulsesPerUnit(); 
}

/*
 * Set the number of units displaced per motor revolution of the motor driving this axis
 * value The setting value in units
 */

void AxisSettings::SetUnitsPerRevolution(int32_t value)
{
    unitsPerRevolution = static_cast<float>(value);
    ComputePulsesPerUnit(); 
}

/*
 * Set the maximum allowable jerk during acceleration/deceleration
 * value The setting value in units/minute^3/1e6
 */

void AxisSettings::SetMaxJerk(int32_t value)
{
    maxJerk = static_cast<float>(value) * 1e6f;
}

/*
 * Set the target speed for movements 
 * value The setting value in units/minute
 */

void AxisSettings::SetSpeed(int32_t value)
{
    speed = static_cast<float>(value);
}

/*
 * Set the maximum allowable speed for movements
 * value The setting value in units/minute
 */

void AxisSettings::SetMaxSpeed(int32_t value)
{
    maxSpeed = static_cast<float>(value);
}

/*
 * Set the microstepping mode for the motor driving this axis
 * value A flag determining the microstepping mode to use
 *          1 = full step, 2 = half step, ... 6 = 1/32 step
 */

void AxisSettings::SetMicrosteppingMode(uint8_t value)
{
    // Compute the number of motor steps per microsteps from the flag
    // microstepping factor = 2 ^ (mode flag - 1)
    microsteppingFactor = 1 << (value - 1);
  
    ComputePulsesPerUnit(); 
}

/*
 * Return the number of pulses required to move this axis by one unit
 */

float AxisSettings::PulsesPerUnit() const
{
    return pulsesPerUnit;
}

/*
 * Return the maximum allowable jerk during acceleration for this axis in units/minute^3
 */

float AxisSettings::MaxJerk() const
{
    return maxJerk;
}

/*
 * Return the current speed value
 */

float AxisSettings::Speed() const
{
    return speed;
}

/*
 * Return the current maximum speed value
 */

float AxisSettings::MaxSpeed() const
{
    return maxSpeed;
}

/*
 * Update the number of pulses to move one unit using current setting values
 */

void AxisSettings::ComputePulsesPerUnit()
{
    pulsesPerUnit = (360 * microsteppingFactor) / (stepAngle * unitsPerRevolution);
}
