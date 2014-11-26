/* 
 * File:   main.cpp
 * Author: Richard Greene
 * zed utility for sending commands to motorboard
 * (most of this code copied from 3dp2)
 * Created on April 8, 2014, 5:20 PM
 */

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include <Hardware.h>
#include <Motor.h>

int inputPin=60;  //P9-12
char GPIOInputValue[64];
FILE *inputHandle = NULL;
bool useMotors = true;

long getMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
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
char getPinInput()
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

        usleep (100000);
        
        if(getValue[0] == '1')
        {
            usleep (2000000);
            break;
        }
     }
 
  return('@');  
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
  
    usleep(1000000);
    if(!cmdLine)
        printf("sending @\n");
    motor.Write(MOTOR_COMMAND, ACK) ;

    unsigned int then = getMillis () + 5000 ;
    while (getMillis () < then)
      if (getPinInput() == ACK)
          break ;
    
    if (getMillis () >= then)
    {
      printf ("zed: Can't establish comms with motor board\n") ;
      exit (EXIT_FAILURE) ;
    }
    if(!cmdLine)
        printf("got interrupt\n");
    
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
        
        if(strlen(cmd) > 1)
            motor.Write(MOTOR_COMMAND, (const unsigned char*)cmd);
        else
            motor.Write(MOTOR_COMMAND, cmd[0]);
    
        if(!cmdLine)
            printf("awaiting ACK\n");
        if (getPinInput() != ACK)
        {
          printf ("zed: motor board didn't ACK command\n") ;
          exit (EXIT_FAILURE) ;
        }
        
        if(cmdLine)
            break;
    }
    
    return 0;
}

