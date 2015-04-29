
/* 
 * File:   Motor.cpp
 * Author: Richard Greene
 * 
 * Interfaces with a motor controller via I2C
 * 
 * Created on March 13, 2014, 5:51 PM
 */

#include <Motor.h>
#include <MotorController.h>
#include <Settings.h>

// The motor speed settings are defined in units of RPM and microns/s.
// Multiplying by these conversion factors will convert 
// RPM to degrees/1000/minute and microns/s to microns/minute.
#define R_SPEED_FACTOR (360000)
#define Z_SPEED_FACTOR (60)

/// Public constructor, base class opens I2C connection and sets slave address
Motor::Motor(unsigned char slaveAddress) :
I2C_Device(slaveAddress)
{
}

/// Disables motors (base class closes I2C connection)
Motor::~Motor() 
{
    DisableMotors();
}    

/// Send a single command to the motor controller.  
bool Motor::SendCommand(MotorCommand command)
{
    return command.Send(this);
}

/// Send a set of commands to the motor controller.  Returns false immediately 
/// if any of the commands cannot be sent.
bool Motor::SendCommands(std::vector<MotorCommand> commands)
{
    for(int i = 0; i < commands.size(); i++)
        if(!commands[i].Send(this))
            return false;
    return true;
}

/// Enable (engage) both motors.  Return false if either can't be enabled.
bool Motor::EnableMotors()
{
    return(MotorCommand(MC_ROT_ACTION_REG, MC_ENABLE).Send(this) &&
           MotorCommand(MC_Z_ACTION_REG, MC_ENABLE).Send(this));
}

/// Disable (disengage) both motors.  Return false if either can't be disabled.
bool Motor::DisableMotors()
{
    return(MotorCommand(MC_ROT_ACTION_REG, MC_DISABLE).Send(this) &&
           MotorCommand(MC_Z_ACTION_REG, MC_DISABLE).Send(this));    
}

/// Pause the current motor command(s) in progress (if any).
bool Motor::Pause()
{
    return(MotorCommand(MC_GENERAL_REG, MC_PAUSE).Send(this));
}

/// Resume the  motor command(s) pending at last pause (if any).
bool Motor::Resume()
{
    return(MotorCommand(MC_GENERAL_REG, MC_RESUME).Send(this));
}

/// Clear pending motor command(s).  Typical use would be after a pause, to
/// implement a cancel.
bool Motor::ClearPendingCommands()
{
    return(MotorCommand(MC_GENERAL_REG, MC_CLEAR).Send(this));
}

/// Reset and initialize the motor controller.
bool Motor::Initialize()
{    
    std::vector<MotorCommand> commands;
    
    // perform a software reset
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_RESET));
    
    // set up parameters applying to all Z motions
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_STEP_ANGLE,
                                    SETTINGS.GetInt(Z_STEP_ANGLE)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_UNITS_PER_REV, 
                                    SETTINGS.GetInt(Z_MICRONS_PER_REV)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_MICROSTEPPING, 
                                    SETTINGS.GetInt(Z_MICRO_STEP)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_MAX_SPEED, 
                   Z_SPEED_FACTOR * SETTINGS.GetInt(Z_MAX_SPEED)));

    // set up parameters applying to all rotations
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_STEP_ANGLE, 
                                    SETTINGS.GetInt(R_STEP_ANGLE)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_UNITS_PER_REV, 
                                    SETTINGS.GetInt(R_MILLIDEGREES_PER_REV)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_MICROSTEPPING, 
                                    SETTINGS.GetInt(R_MICRO_STEP)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_MAX_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_MAX_SPEED)));

    // enable the motors
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_ENABLE));
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_ENABLE));
    
    // no interrupt is needed here since no movement was requested 
    return SendCommands(commands);        
}


/// Move the motors to their home position, with optional interrupt such that
/// it may be chained with GoToStartPosition() with only a single interrupt at 
/// the end of both.
bool Motor::GoHome(bool withInterrupt)
{
    std::vector<MotorCommand> commands;
    
    // set rotation parameters
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_HOMING_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_HOMING_SPEED)));
           
    // rotate to the home position
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_HOME));
    
    // rotate 60 degrees back
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, 
                                    SETTINGS.GetInt(R_HOMING_ANGLE)));
    
    // set Z motion parameters
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK,
                                    SETTINGS.GetInt(Z_HOMING_JERK)));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED,
                   Z_SPEED_FACTOR * SETTINGS.GetInt(Z_HOMING_SPEED)));
                                               
    // go to the Z axis upper limit, i.e the home position
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_HOME));
       
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

/// Goes to home position (without interrupt), then lowers the build platform to
/// the PDMS in order to calibrate and/or start a print
bool Motor::GoToStartPosition()
{
    GoHome(false);
    
    std::vector<MotorCommand> commands;
    
    // set rotation parameters
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    SETTINGS.GetInt(R_START_PRINT_JERK)));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                   R_SPEED_FACTOR * SETTINGS.GetInt(R_START_PRINT_SPEED)));
           
    // rotate to the start position
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE,
                                    SETTINGS.GetInt(R_START_PRINT_ANGLE)));
    
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

/// Separate the current layer and go to the position for the next layer. 
bool Motor::GoToNextLayer(LayerType currentLayerType)
{
    int rSeparationJerk;
    int rSeparationSpeed;
    int rotation;
    int zSeparationJerk;
    int zSeparationSpeed;
    int deltaZ;
    int rApproachJerk;
    int rApproachSpeed;
    int zApproachJerk;
    int zApproachSpeed;
        
    // get the parameters for the current type of layer
    switch(currentLayerType)
    {
        case First:
            rSeparationJerk = SETTINGS.GetInt(FL_SEPARATION_R_JERK);
            rSeparationSpeed = SETTINGS.GetInt(FL_SEPARATION_R_SPEED);
            rotation = SETTINGS.GetInt(FL_ROTATION);
            zSeparationJerk = SETTINGS.GetInt(FL_SEPARATION_Z_JERK);
            zSeparationSpeed = SETTINGS.GetInt(FL_SEPARATION_Z_SPEED);
            deltaZ = SETTINGS.GetInt(FL_Z_LIFT);
            rApproachJerk = SETTINGS.GetInt(FL_APPROACH_R_JERK);
            rApproachSpeed = SETTINGS.GetInt(FL_APPROACH_R_SPEED);
            zApproachJerk = SETTINGS.GetInt(FL_APPROACH_Z_JERK);
            zApproachSpeed = SETTINGS.GetInt(FL_APPROACH_Z_SPEED);
            break;
            
        case BurnIn:
            rSeparationJerk = SETTINGS.GetInt(BI_SEPARATION_R_JERK);
            rSeparationSpeed = SETTINGS.GetInt(BI_SEPARATION_R_SPEED);
            rotation = SETTINGS.GetInt(BI_ROTATION);
            zSeparationJerk = SETTINGS.GetInt(BI_SEPARATION_Z_JERK);
            zSeparationSpeed = SETTINGS.GetInt(BI_SEPARATION_Z_SPEED);
            deltaZ = SETTINGS.GetInt(BI_Z_LIFT);
            rApproachJerk = SETTINGS.GetInt(BI_APPROACH_R_JERK);
            rApproachSpeed = SETTINGS.GetInt(BI_APPROACH_R_SPEED);
            zApproachJerk = SETTINGS.GetInt(BI_APPROACH_Z_JERK);
            zApproachSpeed = SETTINGS.GetInt(BI_APPROACH_Z_SPEED);
            break;
            
        case Model:
            rSeparationJerk = SETTINGS.GetInt(ML_SEPARATION_R_JERK);
            rSeparationSpeed = SETTINGS.GetInt(ML_SEPARATION_R_SPEED);
            rotation = SETTINGS.GetInt(ML_ROTATION);
            zSeparationJerk = SETTINGS.GetInt(ML_SEPARATION_Z_JERK);
            zSeparationSpeed = SETTINGS.GetInt(ML_SEPARATION_Z_SPEED);
            deltaZ = SETTINGS.GetInt(ML_Z_LIFT);
            rApproachJerk = SETTINGS.GetInt(ML_APPROACH_R_JERK);
            rApproachSpeed = SETTINGS.GetInt(ML_APPROACH_R_SPEED);
            zApproachJerk = SETTINGS.GetInt(ML_APPROACH_Z_JERK);
            zApproachSpeed = SETTINGS.GetInt(ML_APPROACH_Z_SPEED);
            break;
    }
        
    rSeparationSpeed *= R_SPEED_FACTOR;
    zSeparationSpeed *= Z_SPEED_FACTOR;
    rApproachSpeed   *= R_SPEED_FACTOR;
    zApproachSpeed   *= Z_SPEED_FACTOR;

    std::vector<MotorCommand> commands;

    // rotate the previous layer from the PDMS
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    rSeparationJerk));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                                    rSeparationSpeed));
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, 
                                    rotation));
    // lift the build platform
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK, 
                                    zSeparationJerk));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                    zSeparationSpeed));
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, deltaZ));
    
    // rotate back to the PDMS
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_JERK, 
                                    rApproachJerk));
    commands.push_back(MotorCommand(MC_ROT_SETTINGS_REG, MC_SPEED, 
                                    rApproachSpeed));
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE,  -rotation));
    
    // lower into position to expose the next layer
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_JERK, 
                                    zApproachJerk));
    commands.push_back(MotorCommand(MC_Z_SETTINGS_REG, MC_SPEED, 
                                    zApproachSpeed));
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                    SETTINGS.GetInt(LAYER_THICKNESS) - deltaZ));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

/// Rotate the tray and lift the build head to inspect the print in progress.
bool Motor::PauseAndInspect(int rotation)
{
    // assume speeds & jerks have already 
    // been set as needed for approach from the current layer type 
    
    std::vector<MotorCommand> commands;

    // rotate the tray to cover stray light from the projector
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, -rotation));
    
    // lift the build head for inspection
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                    SETTINGS.GetInt(INSPECTION_HEIGHT)));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

/// Rotate the tray and lower the build head from the inspection position,
/// to resume printing. 
bool Motor::ResumeFromInspect(int rotation)
{
    // assumes speeds & jerks have already 
    // been set as needed for approach from the current layer type 
    
    std::vector<MotorCommand> commands;

    // rotate the tray back into exposing position
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, rotation));
    
    // lower the build head for exposure
    commands.push_back(MotorCommand(MC_Z_ACTION_REG, MC_MOVE, 
                                    -SETTINGS.GetInt(INSPECTION_HEIGHT)));
    
    // request an interrupt when these commands are completed
    commands.push_back(MotorCommand(MC_GENERAL_REG, MC_INTERRUPT));
    
    return SendCommands(commands);
}

/// Attempt to recover from a jam by homing the build tray.  It's up to the 
/// caller to determine if the anti-jam sensor is successfully triggered
/// during the attempt.
bool Motor::TryJamRecovery(LayerType currentLayerType)
{
    // assumes speed & jerk have already 
    // been set as needed for approach from the current layer type 

    std::vector<MotorCommand> commands;
               
    // rotate to the home position
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_HOME));
    
    // rotate 60 degrees back
    commands.push_back(MotorCommand(MC_ROT_ACTION_REG, MC_MOVE, 
                                    SETTINGS.GetInt(R_HOMING_ANGLE)));
    
    return SendCommands(commands);    
}