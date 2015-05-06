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
bool useMotors = true;

/// Parse input and send appropriate command to motor controller.  Returns true if
/// and only if the command is an interrupt request for which we need to wait.
bool SendCommand(char* cmd)
{
    bool isIRQ = false;
    Motor motor(useMotors ? MOTOR_SLAVE_ADDRESS : 0xFF);

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
                MotorCommand(MC_GENERAL_REG, MC_RESET).Send(&motor);
                break;
                
            case 'C':   // clear
                motor.ClearPendingCommands();
                break;
                
            case 'P':   // pause
                motor.Pause();
                break;
                
            case 'U':   // resume
                motor.Resume();
                
            case 'W':   // request interrupt
                isIRQ = true;
                MotorCommand(MC_GENERAL_REG, MC_INTERRUPT).Send(&motor);
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
                
            case 'F':   // first layer
                isIRQ = true;
                motor.GoToNextLayer(First);
                break;
                    
            case 'B':   // burn-in layer
                isIRQ = true;
                motor.GoToNextLayer(BurnIn);
                break;
                    
            case 'M':   // model layer
                isIRQ = true;
                motor.GoToNextLayer(Model);
                break;
                
            case 'S':   // refresh settings
                SETTINGS.Refresh();
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
        int32_t value;
        
        // find the register
        switch(cmd[0])
        {
            case 'z':
                cmdRegister = MC_Z_SETTINGS_REG;
                break;
                
            case 'r':
                cmdRegister = MC_ROT_SETTINGS_REG;
                break;
                
            case 'Z':
                cmdRegister = MC_Z_ACTION_REG;
                break;
                
            case 'R':
                cmdRegister = MC_ROT_ACTION_REG;
                break;
                
            default:
                printf("Unknown command register: %c\n", cmd[0]);
                return false;
                break;
        }
        
        // find the command
        switch(cmd[1])
        {
            case 'n':   // step angle
                command = MC_STEP_ANGLE;
                break;
                
            case 'u':   // units
                command = MC_UNITS_PER_REV;
                break;
                
            case 't':   // microstep mode
                command = MC_MICROSTEPPING;
                break;
                
            case 'x':   // max speed
                command = MC_MAX_SPEED;
                break;
                
            case 's':   // target speed
                command = MC_SPEED;
                break;
                
            case 'j':   // jerk
                command = MC_JERK;
                break;
                            
            default:
                printf("Unknown command %c for register: %c\n", cmd[1], cmd[0]);
                return false;
                break;
        }
        
        // send the command   
        MotorCommand(cmdRegister, command, atoi(cmd + 2)).Send(&motor);
    }
    return isIRQ;
}

// set up a pin as an input
void setupPinInput()
{
    char setValue[4], GPIOInputString[4], GPIODirection[64];
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
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL){
        printf("Unable to open direction handle\n");
        exit (EXIT_FAILURE) ;
    }
    strcpy(setValue,"in");
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);   
}

// wait for input pin from the motor board going high
// (unless we're not using motors at all))
void getPinInput()
{
    char getValue[4];

    while(useMotors)
     {
        if ((inputHandle = fopen(GPIOInputValue, "rb+")) == NULL){
            printf("Unable to open input handle\n");
            exit (EXIT_FAILURE) ;
        }
        fread(&getValue, sizeof(char), 1, inputHandle);
        fclose(inputHandle);  

        // TODO: see if this delay can be reduced with new AVR FW
        usleep (100000);
        
        if(getValue[0] == '1')
        {
            // TODO: see if this delay can be reduced with new AVR FW
            usleep (2000000);
            break;
        }
     }
 }

/// If a command line argument is given, execute that motor command and exit.
/// Otherwise loop on keyboard input for the command.
int main(int argc, char** argv) {

    // see if we should execute one command from command line
    bool cmdLine = false;
    if(argc > 1) 
    {
        cmdLine = strlen(argv[1]) > 0;
    }
    
    setupPinInput();
    
    char buf[256];
    char *p;
    char *cmd;
    
    while(buf[0] != 3) // do until we get a Ctrl-C
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
            cmd = argv[1];
        
        bool awaitInterrupt = SendCommand(cmd);
    
        if(awaitInterrupt)
        {
            if(!cmdLine)
                printf("awaiting ACK\n");
            getPinInput();
        }
        
        if(cmdLine)
            break;
    }
    
    return 0;
}
