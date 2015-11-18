//  File:   main.cpp
//  utility (zee) for sending commands to motor controller
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

#include <cstdlib>
#include <string.h>

#include <Hardware.h>
#include <Motor.h>
#include <MotorController.h>
#include <Settings.h>
#include <LayerSettings.h>
#include "I2C_Device.h"

using namespace std;

int inputPin = MOTOR_INTERRUPT_PIN;  
char GPIOInputValue[64];
FILE *inputHandle = NULL;
CurrentLayerSettings firstLS;
CurrentLayerSettings burninLS;
CurrentLayerSettings modelLS;
Settings& settings = PrinterSettings::Instance();

// get the current settings, for use by commands that depend on the layer type
void LoadCurrentLayerSettings()
{
    // set one up for First layer
    firstLS.PressMicrons = settings.GetInt(FL_PRESS);
    firstLS.PressMicronsPerSec = settings.GetInt(FL_PRESS_SPEED);
    firstLS.PressWaitMS = settings.GetInt(FL_PRESS_WAIT);
    firstLS.UnpressMicronsPerSec = settings.GetInt(FL_UNPRESS_SPEED);
    firstLS.ApproachWaitMS = settings.GetInt(FL_APPROACH_WAIT);
    firstLS.ExposureSec = settings.GetDouble(FIRST_EXPOSURE);
    firstLS.SeparationRotJerk = settings.GetInt(FL_SEPARATION_R_JERK);
    firstLS.SeparationRPM = settings.GetInt(FL_SEPARATION_R_SPEED);
    firstLS.RotationMilliDegrees = settings.GetInt(FL_ROTATION);
    firstLS.SeparationZJerk = settings.GetInt(FL_SEPARATION_Z_JERK);
    firstLS.SeparationMicronsPerSec = settings.GetInt(FL_SEPARATION_Z_SPEED);
    firstLS.ZLiftMicrons = settings.GetInt(FL_Z_LIFT);
    firstLS.ApproachRotJerk = settings.GetInt(FL_APPROACH_R_JERK);
    firstLS.ApproachRPM = settings.GetInt(FL_APPROACH_R_SPEED);
    firstLS.ApproachZJerk = settings.GetInt(FL_APPROACH_Z_JERK);
    firstLS.ApproachMicronsPerSec = settings.GetInt(FL_APPROACH_Z_SPEED);
    firstLS.LayerThicknessMicrons = settings.GetInt(LAYER_THICKNESS);

    burninLS.PressMicrons = settings.GetInt(BI_PRESS);
    burninLS.PressMicronsPerSec = settings.GetInt(BI_PRESS_SPEED);
    burninLS.PressWaitMS = settings.GetInt(BI_PRESS_WAIT);
    burninLS.UnpressMicronsPerSec = settings.GetInt(BI_UNPRESS_SPEED);
    burninLS.ApproachWaitMS = settings.GetInt(BI_APPROACH_WAIT);
    burninLS.ExposureSec = settings.GetDouble(BURN_IN_EXPOSURE);
    burninLS.SeparationRotJerk = settings.GetInt(BI_SEPARATION_R_JERK);
    burninLS.SeparationRPM = settings.GetInt(BI_SEPARATION_R_SPEED);
    burninLS.RotationMilliDegrees = settings.GetInt(BI_ROTATION);
    burninLS.SeparationZJerk = settings.GetInt(BI_SEPARATION_Z_JERK);
    burninLS.SeparationMicronsPerSec = settings.GetInt(BI_SEPARATION_Z_SPEED);
    burninLS.ZLiftMicrons = settings.GetInt(BI_Z_LIFT);
    burninLS.ApproachRotJerk = settings.GetInt(BI_APPROACH_R_JERK);
    burninLS.ApproachRPM = settings.GetInt(BI_APPROACH_R_SPEED);
    burninLS.ApproachZJerk = settings.GetInt(BI_APPROACH_Z_JERK);
    burninLS.ApproachMicronsPerSec = settings.GetInt(BI_APPROACH_Z_SPEED);
    burninLS.LayerThicknessMicrons = settings.GetInt(LAYER_THICKNESS);

    modelLS.PressMicrons = settings.GetInt(ML_PRESS);
    modelLS.PressMicronsPerSec = settings.GetInt(ML_PRESS_SPEED);
    modelLS.PressWaitMS = settings.GetInt(ML_PRESS_WAIT);
    modelLS.UnpressMicronsPerSec = settings.GetInt(ML_UNPRESS_SPEED);
    modelLS.ApproachWaitMS = settings.GetInt(ML_APPROACH_WAIT);
    modelLS.ExposureSec = settings.GetDouble(MODEL_EXPOSURE); 
    modelLS.SeparationRotJerk = settings.GetInt(ML_SEPARATION_R_JERK);
    modelLS.SeparationRPM = settings.GetInt(ML_SEPARATION_R_SPEED);
    modelLS.RotationMilliDegrees = settings.GetInt(ML_ROTATION);
    modelLS.SeparationZJerk = settings.GetInt(ML_SEPARATION_Z_JERK);
    modelLS.SeparationMicronsPerSec = settings.GetInt(ML_SEPARATION_Z_SPEED);
    modelLS.ZLiftMicrons = settings.GetInt(ML_Z_LIFT);
    modelLS.ApproachRotJerk = settings.GetInt(ML_APPROACH_R_JERK);
    modelLS.ApproachRPM = settings.GetInt(ML_APPROACH_R_SPEED);
    modelLS.ApproachZJerk = settings.GetInt(ML_APPROACH_Z_JERK);
    modelLS.ApproachMicronsPerSec = settings.GetInt(ML_APPROACH_Z_SPEED);
    modelLS.LayerThicknessMicrons = settings.GetInt(LAYER_THICKNESS);
}

/// Parse input and send appropriate command to motor controller.  Returns true 
/// if and only if the command includes an interrupt request for which we need 
/// to wait.
bool SendCommand(char* cmd, Motor& motor, const I_I2C_Device& i2cDevice)
{
    bool isIRQ = false;

    if(strlen(cmd) < 1)
    {
        printf("Empty command");
        return false;
    }
    
    if(strlen(cmd) == 1)
    {
        // handle singe-character commands
        switch(cmd[0])
        {
            case 'T':   // reset
                MotorCommand(MC_GENERAL_REG, MC_RESET).Send(i2cDevice);
                break;
                
            case 'C':   // clear
                motor.ClearPendingCommands();
                break;
                
            case 'P':   // pause
                motor.Pause();
                break;
                
            case 'U':   // resume
                motor.Resume();
                break;
                
            case 'W':   // request interrupt
                isIRQ = true;
                MotorCommand(MC_GENERAL_REG, MC_INTERRUPT).Send(i2cDevice);
                break;
                
            case 'I':   // initialize
                motor.Initialize();
                break;
                
            case 'H':   // home
                isIRQ = true;
                motor.GoHome();
                break;
                
            case 'G':   // start/calibration position
                isIRQ = true;
                motor.GoToStartPosition();
                break;
                
            case 'F':   // first layer separation
                isIRQ = true;
                motor.Separate(firstLS);
                break;
                    
            case 'B':   // burn-in layer separation
                isIRQ = true;
                motor.Separate(burninLS);
                break;
                    
            case 'M':   // model layer separation
                isIRQ = true;
                motor.Separate(modelLS);
                break;
                
            case 'f':   // first layer approach
                isIRQ = true;
                motor.Approach(firstLS);
                break;
                    
            case 'b':   // burn-in layer approach
                isIRQ = true;
                motor.Approach(burninLS);
                break;
                    
            case 'm':   // model layer approach
                isIRQ = true;
                motor.Approach(modelLS);
                break;
                
            case 'S':   // refresh settings
                settings.Refresh();
                LoadCurrentLayerSettings();
                break;
                
            case 'E':   // enable
                MotorCommand(MC_GENERAL_REG, MC_ENABLE).Send(i2cDevice);
                break;

            case 'D':   // disable
                MotorCommand(MC_GENERAL_REG, MC_DISABLE).Send(i2cDevice);                
                break;
                    
            default:
                printf("Unknown command: %c\n", cmd[0]);
                break;
        }
    }
    else    // we have a multiple-character command with at least two chars  
    {
        unsigned char cmdRegister;
        unsigned char command;
        int32_t value = atoi(cmd + 2);
        int speedFactor;  // for appropriately scaling units
        int distanceFactor = 1;  
        
        // find the register
        switch(cmd[0])
        {
            case 'z':
                cmdRegister = MC_Z_SETTINGS_REG;
                speedFactor = Z_SPEED_FACTOR;
                break;
                
            case 'r':
                cmdRegister = MC_ROT_SETTINGS_REG;
                speedFactor = R_SPEED_FACTOR;
                distanceFactor = R_SCALE_FACTOR;
                break;
                
            case 'Z':
                cmdRegister = MC_Z_ACTION_REG;
                break;
                
            case 'R':
                cmdRegister = MC_ROT_ACTION_REG;
                distanceFactor = R_SCALE_FACTOR;
                break;
                
            default:
                printf("Unknown command register: %c\n", cmd[0]);
                return false;
                break;
        }
        
        // find the command for settings registers
        if(cmdRegister == MC_Z_SETTINGS_REG || 
           cmdRegister == MC_ROT_SETTINGS_REG)
        {
            switch(cmd[1])
            {
                case 'n':   // step angle
                    command = MC_STEP_ANGLE;
                    break;

                case 'u':   // units
                    command = MC_UNITS_PER_REV;
                    value /= distanceFactor;
                    break;

                case 't':   // microstep mode
                    command = MC_MICROSTEPPING;
                    break;

                case 's':   // target speed
                    command = MC_SPEED;
                    value *= speedFactor;
                    break;

                case 'j':   // jerk
                    command = MC_JERK;
                    break;

                default:
                    printf("Unknown command %c for register: %c\n", cmd[1], 
                                                                    cmd[0]);
                    return false;
                    break;
            }
        }
        else    // find the command for action registers
        {
            switch(cmd[1])
            {
                case 'V':   // move
                    command = MC_MOVE;
                    value /= distanceFactor;
                    break;

                case 'L':   // home
                    command = MC_HOME;
                    value /= distanceFactor;
                    break;

                default:
                    printf("Unknown command %c for register: %c\n", cmd[1], 
                                                                    cmd[0]);
                    return false;
                    break;
            }
        }
        // send the command   
        MotorCommand(cmdRegister, command, value).Send(i2cDevice);
    }
    return isIRQ;   // return false here to disable waiting for interrupts
}

// set up a pin as an input
void setupPinInput()
{
    char setValue[4];
    char GPIOInputString[4];
    char GPIODirection[64];
    
    // setup input
    sprintf(GPIOInputString, "%d", inputPin);
    sprintf(GPIOInputValue, "/sys/class/gpio/gpio%d/value", inputPin);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", inputPin);
 
    // Export the pin
    if ((inputHandle = fopen("/sys/class/gpio/export", "ab")) == NULL){
        printf("Unable to export GPIO pin\n");
        exit (EXIT_FAILURE) ;
    }
    strcpy(setValue, GPIOInputString);
    fwrite(setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL){
        printf("Unable to open direction handle\n");
        exit (EXIT_FAILURE) ;
    }
    strcpy(setValue,"in");
    fwrite(setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);  
}

// wait for input pin from the motor board going low and high again
// (unless we're not using motors at all))
void getPinInput()
{
    
    char getValue = 'x';
    bool foundFalling = false;
    
    while(true)
    {
        if ((inputHandle = fopen(GPIOInputValue, "rb")) == NULL)
        {
            printf("Unable to open input handle\n");
            exit (EXIT_FAILURE) ;
        }
        fread(&getValue, sizeof(char), 1, inputHandle);
        fclose(inputHandle);  
        
        if(!foundFalling)
        {
            if(getValue == '0')
                foundFalling = true;
        }
        else if(getValue == '1')
            break;  // don't exit till rising edge found after falling edge
        
        // wait a bit before trying again
        usleep(1000);
    }
}

/// If a command line argument is given, execute that motor command and exit.
/// Otherwise loop on keyboard input for the command.
int main(int argc, char** argv) {

    // see if we should execute one command from command line
    bool cmdLine = false;
    if(argc > 1) 
    {
        cmdLine = argc > 1;
    }
    
    setupPinInput();
    I2C_Device i2cDevice(MOTOR_SLAVE_ADDRESS, I2C2_PORT);
    Motor motor(i2cDevice);
    
    char buf[256];
    char *p;
    char *cmd;
    
    int arg = 1;
    
    LoadCurrentLayerSettings();
    
    // do until we get a Ctrl-C, or no more args if running from command line
    while(buf[0] != 3 && (!cmdLine || arg < argc)) 
    {
        if(!cmdLine)
        {
            printf("Enter motor command:\n");
            // get string from keyboard into buf
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
              if ((p = strchr(buf, '\n')) != NULL)
                *p = '\0';
            }

            cmd = buf;
        }
        else
            cmd = argv[arg++];
        
        bool awaitInterrupt = SendCommand(cmd, motor, i2cDevice);
    
        if(awaitInterrupt)
        {
            if(!cmdLine)
                printf("awaiting ACK\n");
            getPinInput();
        }
    }
    
    // don't call Motor d'tor, so that motors won't be disabled on exit
    
    return 0;
}
