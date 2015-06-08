/*
 * AxisSettings.h
 * Author: Jason Lefley
 * Date  : 2015-04-28
 */

#ifndef AXISSETTINGS_H
#define AXISSETTINGS_H

#include <stdint.h>

#include "Status.h"

/*
 * Default setting values
 */

#define AXIS_SETTINGS_DEFAULT_STEP_ANGLE           0.0
#define AXIS_SETTINGS_DEFAULT_UNITS_PER_REVOLUTION 0.0
#define AXIS_SETTINGS_DEFAULT_MAX_JERK             0.0
#define AXIS_SETTINGS_DEFAULT_SPEED                0.0
#define AXIS_SETTINGS_DEFAULT_MICROSTEPPING_FACTOR 0

class AxisSettings
{
public:
    AxisSettings();
    ~AxisSettings();

    // Set settings
    Status SetStepAngle(int32_t value);
    Status SetUnitsPerRevolution(int32_t value);
    Status SetMaxJerk(int32_t value);
    Status SetSpeed(int32_t value);
    Status SetMicrosteppingMode(uint8_t value);

    // Retrieve settings
    float PulsesPerUnit() const;
    float MaxJerk() const;
    float Speed() const;

    Status Validate() const;
   
private:
    AxisSettings(const AxisSettings&);

private:
    // Initialize settings with defaults here
    float stepAngle = AXIS_SETTINGS_DEFAULT_STEP_ANGLE;
    float unitsPerRevolution = AXIS_SETTINGS_DEFAULT_UNITS_PER_REVOLUTION;
    float maxJerk = AXIS_SETTINGS_DEFAULT_MAX_JERK;
    float speed = AXIS_SETTINGS_DEFAULT_SPEED;
    uint8_t microsteppingFactor = AXIS_SETTINGS_DEFAULT_MICROSTEPPING_FACTOR;
};

#endif /* AXISSETTINGS_H */
