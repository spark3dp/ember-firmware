//  File:   Motor.cpp
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

#include <Motor.h>
#include <MotorController.h>
#include <Settings.h>

#define DELAY_AFTER_RESET_MSEC  (500)
#define USE_HOMING_FOR_APPROACH (-1) 

// Public constructor, base class opens I2C connection and sets slave address
Motor::Motor(unsigned char slaveAddress) :
I2C_Device(slaveAddress)
{
}

// Disables motors (base class closes I2C connection)
Motor::~Motor() 
{
    DisableMotors();
}    

// Send a set of commands to the motor controller.  Returns false immediately 
// if any of the commands cannot be sent.
bool Motor::SendCommands(std::vector<MotorCommand> commands)
{
    for(int i = 0; i < commands.size(); i++)
        if (!commands[i].Send(this))
            return false;
    return true;
}

// Enable (engage) both motors.  Return false if they can't be enabled.
bool Motor::EnableMotors()
{
    return MotorCommand(MC_GENERAL_REG, MC_ENABLE).Send(this);
}

// Disable (disengage) both motors.  Return false if they can't be disabled.
bool Motor::DisableMotors()
{
    return MotorCommand(MC_GENERAL_REG, MC_DISABLE).Send(this);    
}

// Pause the current motor command(s) in progress (if any).
bool Motor::Pause()
{
    return MotorCommand(MC_GENERAL_REG, MC_PAUSE).Send(this);
}

// Resume the  motor command(s) pending at last pause (if any).
bool Motor::Resume()
{
    return MotorCommand(MC_GENERAL_REG, MC_RESUME).Send(this);
}

// Clear pending motor command(s).  Used when canceling a print, after a pause.
// Interrupt should be requested if the motor may not have completed the 
// preceding pause yet.
bool Motor::ClearPendingCommands(bool withInterrupt)
{
    std::vector<MotorCommand> commands;
    
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_CLEAR));
    
    if (withInterrupt)
    {
        // request an interrupt, to avoid sending new motor commands before the 
        // clear has been completed
        commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    }
    
    return SendCommands(commands);  
}

// Reset and initialize the motor controller.
bool Motor::Initialize()
{    
    std::vector<MotorCommand> commands;
    
    // perform a software reset
    if (!MotorCommand(MC_GENERAL_REG, MC_RESET).Send(this))
        return false;
    
    // wait for the reset to complete before sending any commands
    // (that would otherwise be erased as part of the reset)
    usleep(DELAY_AFTER_RESET_MSEC * 1000);
    
    // set up parameters applying to all Z motions
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_STEP_ANGLE,
                                    SETTINGS.GetInt(Z_STEP_ANGLE)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_UNITS_PER_REV, 
                                    SETTINGS.GetInt(Z_MICRONS_PER_REV)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_MICROSTEPPING, 
                                    SETTINGS.GetInt(MICRO_STEPS_MODE)));

    // set up parameters applying to all rotations
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_STEP_ANGLE, 
                                    SETTINGS.GetInt(R_STEP_ANGLE)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_UNITS_PER_REV, 
                   SETTINGS.GetInt(R_MILLIDEGREES_PER_REV) / R_SCALE_FACTOR));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_MICROSTEPPING, 
                                    SETTINGS.GetInt(MICRO_STEPS_MODE)));

    // enable the motors
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_ENABLE));
    
    // no interrupt is needed here since no movement was requested 
    return SendCommands(commands);        
}


// Move the motors to their home position, with optional interrupt such that
// it may be chained with GoToStartPosition() with only a single interrupt at 
// the end of both.  Also with option to kep the tray's window in the open
// position, in support of demo mode.
bool Motor::GoHome(bool withInterrupt, bool stayOpen)
{
    std::vector<MotorCommand> commands;
    
    // set rotation parameters
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_HOMING_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_HOMING_SPEED)));
           
    // rotate to the home position (but no more than a full rotation)
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_HOME,
                                    UNITS_PER_REVOLUTION));
    
    int homeAngle = SETTINGS.GetInt(R_HOMING_ANGLE) / R_SCALE_FACTOR;
    if (homeAngle != 0 && !stayOpen)
    {
        // rotate 60 degrees back
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, homeAngle));
    }
    
    // set Z motion parameters
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK,
                                    SETTINGS.GetInt(Z_HOMING_JERK)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED,
                   Z_SPEED_FACTOR * SETTINGS.GetInt(Z_HOMING_SPEED)));
                                               
    // go up to the Z home position (but no more than twice the max Z travel)
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_HOME,
                               -2 * SETTINGS.GetInt(Z_START_PRINT_POSITION)));
     
    if (withInterrupt)
    {           
        // request an interrupt when these commands are completed
        commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    }
    return SendCommands(commands);
}

// Goes to home position (without interrupt), then lowers the build platform to
// the PDMS in order to calibrate and/or start a print
bool Motor::GoToStartPosition()
{
    EnableMotors();
    
    GoHome(false);
    
    std::vector<MotorCommand> commands;
    
    // set rotation parameters
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_START_PRINT_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_START_PRINT_SPEED)));
      
    int startAngle = SETTINGS.GetInt(R_START_PRINT_ANGLE) / R_SCALE_FACTOR;
    if (startAngle != 0)
    {
        // rotate to the start position
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, 
                                                                startAngle));
    }
    
    // set Z motion parameters
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK,
                                    SETTINGS.GetInt(Z_START_PRINT_JERK)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED,
                   Z_SPEED_FACTOR * SETTINGS.GetInt(Z_START_PRINT_SPEED)));

    // move down to the PDMS
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                    SETTINGS.GetInt(Z_START_PRINT_POSITION)));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Separate the current layer 
bool Motor::Separate(const CurrentLayerSettings& cls)
{
    std::vector<MotorCommand> commands;

    // rotate the previous layer from the PDMS
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    cls.SeparationRotJerk));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                                    cls.SeparationRPM * R_SPEED_FACTOR));
    
    int rotation = cls.RotationMilliDegrees / R_SCALE_FACTOR;
    if (rotation != 0)
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, -rotation));
    
    // lift the build platform
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK, 
                                    cls.SeparationZJerk));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                 cls.SeparationMicronsPerSec * Z_SPEED_FACTOR));
    
    if (cls.ZLiftMicrons != 0)
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                                            cls.ZLiftMicrons));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Go to the position for exposing the next layer (with optional jam recovery
// motion first). 
bool Motor::Approach(const CurrentLayerSettings& cls, bool unJamFirst)
{
    if (unJamFirst)
        if (!UnJam(cls, false))
            return false;

    std::vector<MotorCommand> commands;
    
    // rotate back to the PDMS
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    cls.ApproachRotJerk));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                                    cls.ApproachRPM * R_SPEED_FACTOR));
    
    int rotation = cls.RotationMilliDegrees / R_SCALE_FACTOR;
    if (rotation != 0)
    {
        // see if we should use homing on approach, to avoid not rotating far 
        // enough back when there's been drag (a partial jam) on separation
        if (SETTINGS.GetInt(HOME_ON_APPROACH) != 0)
            commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_HOME, 
                                                                 2 * rotation));
        else
            commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, 
                                                                     rotation));
    }
    
    // lower into position to expose the next layer
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK, 
                                    cls.ApproachZJerk));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                   cls.ApproachMicronsPerSec * Z_SPEED_FACTOR));
    
    int deltaZ = cls.LayerThicknessMicrons - cls.ZLiftMicrons;
    if (deltaZ != 0)
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, deltaZ));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Rotate the tray and (if CanInspect is true) lift the build head to inspect 
// the print in progress.
bool Motor::PauseAndInspect(const CurrentLayerSettings& cls)
{    
    std::vector<MotorCommand> commands;
    
    // use same speeds & jerks as used for homing, since we're already separated     
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_HOMING_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_HOMING_SPEED)));

    // rotate the tray to cover stray light from the projector
    int rotation = cls.RotationMilliDegrees / R_SCALE_FACTOR;
    if (rotation != 0)
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, -rotation));
    
    if (cls.CanInspect)
    {
        commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK,
                                        SETTINGS.GetInt(Z_HOMING_JERK)));
        commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED,
                       Z_SPEED_FACTOR * SETTINGS.GetInt(Z_HOMING_SPEED)));

        // lift the build head for inspection
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                                  cls.InspectionHeightMicrons));
    }
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Rotate the tray and (if CanInspect is true) lower the build head from the 
// inspection position, to resume printing. 
bool Motor::ResumeFromInspect(const CurrentLayerSettings& cls)
{
    std::vector<MotorCommand> commands;

    // use same speeds & jerks as used for moving to start position, 
    // since we're already calibrated     
    // set rotation parameters
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_START_PRINT_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_START_PRINT_SPEED)));
      

    // rotate the tray back into exposing position
    int rotation = cls.RotationMilliDegrees / R_SCALE_FACTOR;
    if (rotation != 0)
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, rotation));
    
    if (cls.CanInspect)
    {
        commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK,
                                        SETTINGS.GetInt(Z_START_PRINT_JERK)));
        commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED,
                       Z_SPEED_FACTOR * SETTINGS.GetInt(Z_START_PRINT_SPEED)));

        // lower the build head for exposure
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                                 -cls.InspectionHeightMicrons));
    }
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Attempt to recover from a jam by homing the build tray.  It's up to the 
// caller to determine if the anti-jam sensor is successfully triggered
// during the attempt.  This move (without the interrupt request)is also 
// required before resuming after a manual recovery, in order first to  
// align the tray correctly.
bool Motor::UnJam(const CurrentLayerSettings& cls, bool withInterrupt)
{
    // assumes speed & jerk have already 
    // been set as needed for separation from the current layer type 

    std::vector<MotorCommand> commands;
               
    // rotate to the home position (but no more than a full rotation)
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_HOME,
                                    UNITS_PER_REVOLUTION));
    
    int rotation = cls.RotationMilliDegrees / R_SCALE_FACTOR;
    if (rotation != 0)      
        commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, -rotation));
  
    if (withInterrupt)
    {
        // request an interrupt when these commands are completed
        commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    }

    return SendCommands(commands);    
}

// Press the build head down onto the tray, to deflect it below its resting 
// position.
bool Motor::Press(const CurrentLayerSettings& cls)
{
    // reuse existing jerk settings from approach

    std::vector<MotorCommand> commands;
    
    // press down on the tray
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                    cls.PressMicronsPerSec * Z_SPEED_FACTOR));
    if (cls.PressMicrons != 0)
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                                        -cls.PressMicrons));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

// Move the tray back up into position for exposing the next layer, allowing
// resin to fill in for the height of a full layer. 
bool Motor::Unpress(const CurrentLayerSettings& cls)
{
    // reuse existing jerk settings from approach

    std::vector<MotorCommand> commands;
    
    // lift up on the tray
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                    cls.UnpressMicronsPerSec * Z_SPEED_FACTOR));
    if (cls.PressMicrons != 0)
        commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                                            cls.PressMicrons));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}
