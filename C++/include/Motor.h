//  File:   Motor.h
//  Interfaces with a motor controller via I2C
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef MOTOR_H
#define	MOTOR_H

#include <vector>

#include <MotorCommand.h>
#include <PrinterStatus.h>
#include <LayerSettings.h>
#include <Settings.h>

// Native motor controller rotation units are 1/10 degree or deci-degrees,
// while smith uses 1/1000 degree or milli-degrees.  
// Thus all rotation amounts must be divided by the following scale factor 
// before being sent to the motor controller.
constexpr int R_SCALE_FACTOR = 100;

constexpr int UNITS_PER_REVOLUTION = 360 * 10; // deci-degrees in a full circle
// The motor speed settings are defined in units of RPM and microns/s.
// Multiplying by these conversion factors will convert 
// RPM to degrees/10/minute and microns/s to microns/minute.
constexpr int R_SPEED_FACTOR = UNITS_PER_REVOLUTION;
constexpr int Z_SPEED_FACTOR = 60;

class I_I2C_Device;

class Motor
{
public:
    Motor(const I_I2C_Device& i2cDevice);
    ~Motor();
    bool Initialize();
    bool EnableMotors();
    bool DisableMotors();
    bool Pause();
    bool Resume();
    bool ClearPendingCommands(bool withInterrupt = false);
    bool GoHome(bool withInterrupt = true, bool rotateHome = true, 
                                           bool stayOpen = false);
    bool GoToStartPosition();
    bool Separate(const CurrentLayerSettings& cls);
    bool Approach(const CurrentLayerSettings& cls, bool unJamFirst = false);
    bool PauseAndInspect(const CurrentLayerSettings& cls);
    bool ResumeFromInspect(const CurrentLayerSettings& cls);
    bool UnJam(const CurrentLayerSettings& cls, bool withInterrupt = true);
    bool Press(const CurrentLayerSettings& cls);
    bool Unpress(const CurrentLayerSettings& cls);
    
private:
    bool SendCommands(std::vector<MotorCommand> commands);

    const I_I2C_Device& _i2cDevice;
    Settings& _settings;
};

#endif    // MOTOR_H

