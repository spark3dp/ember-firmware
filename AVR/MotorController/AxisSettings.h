//  File: AxisSettings.h
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

#ifndef AXISSETTINGS_H
#define AXISSETTINGS_H

#include <stdint.h>

#include "Status.h"

// Default setting values
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

#endif  // AXISSETTINGS_H
