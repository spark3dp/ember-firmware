/* 
 * File:   main.cpp
 * Author: Richard Greene
 * 
 * zee utility for sending commands to motor controller
 *
 * Created on May 6, 2015, 10:53 AM
 */

#include <cstdlib>
#include <string.h>

#include <Hardware.h>
#include <Motor.h>
#include <MotorController.h>
#include <Settings.h>

using namespace std;

int inputPin = MOTOR_INTERRUPT_PIN;  
char GPIOInputValue[64];
FILE *inputHandle = NULL;
Motor* pMotor;

bool useMotors = true;

/// Parse input and send appropriate command to motor controller.  Returns true 
/// if and only if the command includes an interrupt request for which we need 
/// to wait.
bool SendCommand(char* cmd)
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
                MotorCommand(MC_GENERAL_REG, MC_RESET).Send(pMotor);
                break;
                
            case 'C':   // clear
                pMotor->ClearPendingCommands();
                break;
                
            case 'P':   // pause
                pMotor->Pause();
                break;
                
            case 'U':   // resume
                pMotor->Resume();
                break;
                
            case 'W':   // request interrupt
                isIRQ = true;
                MotorCommand(MC_GENERAL_REG, MC_INTERRUPT).Send(pMotor);
                break;
                
            case 'I':   // initialize
                pMotor->Initialize();
                break;
                
            case 'H':   // home
                isIRQ = true;
                pMotor->GoHome();
                break;
                
            case 'G':   // start/calibration position
                isIRQ = true;
                pMotor->GoToStartPosition();
                break;
                
            case 'F':   // first layer separation
                isIRQ = true;
                pMotor->Separate(First);
                break;
                    
            case 'B':   // burn-in layer separation
                isIRQ = true;
                pMotor->Separate(BurnIn);
                break;
                    
            case 'M':   // model layer separation
                isIRQ = true;
                pMotor->Separate(Model);
                break;
                
            case 'f':   // first layer approach
                isIRQ = true;
                pMotor->Approach(First, SETTINGS.GetInt(LAYER_THICKNESS));
                break;
                    
            case 'b':   // burn-in layer approach
                isIRQ = true;
                pMotor->Approach(BurnIn, SETTINGS.GetInt(LAYER_THICKNESS));
                break;
                    
            case 'm':   // model layer approach
                isIRQ = true;
                pMotor->Approach(Model, SETTINGS.GetInt(LAYER_THICKNESS));
                break;
                
            case 'S':   // refresh settings
                SETTINGS.Refresh();
                break;
                
            case 'E':   // enable
                MotorCommand(MC_GENERAL_REG, MC_ENABLE).Send(pMotor);
                break;

            case 'D':   // disable
                MotorCommand(MC_GENERAL_REG, MC_DISABLE).Send(pMotor);                
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

                case 'x':   // max speed
                    command = MC_MAX_SPEED;
                    value *= speedFactor;
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
        MotorCommand(cmdRegister, command, value).Send(pMotor);
    }
    return isIRQ;   // just return false here to disable waiting for interrupts
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

// wait for input pin from the motor board going high
// (unless we're not using motors at all))
void getPinInput()
{
    char getValue;

    while(useMotors)
    {
        if ((inputHandle = fopen(GPIOInputValue, "rb+")) == NULL){
            printf("Unable to open input handle\n");
            exit (EXIT_FAILURE) ;
        }
        fread(&getValue, sizeof(char), 1, inputHandle);
        fclose(inputHandle);  
        
        if(getValue == '1')
            break;
        
        // wait a bit before trying again
        usleep(10000);
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
    pMotor = new Motor(useMotors ? MOTOR_SLAVE_ADDRESS : 0xFF);
    
    char buf[256];
    char *p;
    char *cmd;
    
    int arg = 1;
    
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
        
        bool awaitInterrupt = SendCommand(cmd);
    
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
