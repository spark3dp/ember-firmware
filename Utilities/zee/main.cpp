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
#include <string>

#include <Hardware.h>
#include <Motor.h>

using namespace std;

int inputPin = MOTOR_INTERRUPT_PIN;  
char GPIOInputValue[64];
FILE *inputHandle = NULL;
bool useMotors = true;

/// Parse input and send appropriate command to motor controller.  Returns true if
/// and only if the command is an interrupt request for which we need to wait.
bool SendCommand(string cmd)
{
    bool isIRQ = false;
    
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

    if(!cmdLine)
        printf("Connecting to motor board...\n");
    Motor motor(useMotors ? MOTOR_SLAVE_ADDRESS : 0xFF);
    
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

            printf("sending %s\n", buf);
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
