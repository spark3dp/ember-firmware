//  File:   main.cpp
//  For testing Ember front panel during manufacturing
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <sys/epoll.h> 

#include <I2C_Device.h>

using namespace std;

char GPIOInputValue[64];
FILE *inputHandle = NULL;

// set up front panel interrupt pin as an input
int setupPinInput()
{
    char setValue[10], GPIOInputString[4], GPIODirection[64], GPIOEdge[64];
    // setup input
    sprintf(GPIOInputString, "%d", UI_INTERRUPT_PIN);
    sprintf(GPIOInputValue, "/sys/class/gpio/gpio%d/value", UI_INTERRUPT_PIN);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", UI_INTERRUPT_PIN);
    sprintf(GPIOEdge, "/sys/class/gpio/gpio%d/edge", UI_INTERRUPT_PIN);
 
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
    
    // set it to edge triggered
    if ((inputHandle = fopen(GPIOEdge, "rb+")) == NULL)
    {
        printf("Unable to set edge triggered\n");
        exit (EXIT_FAILURE) ;
    }
    const char* edge = "rising";
    strcpy(setValue, edge);
    fwrite(&setValue, sizeof(char), strlen(edge), inputHandle);
    fclose(inputHandle);

    // Open the file descriptor for the interrupt
    int interruptFD = open(GPIOInputValue, O_RDONLY);
    if(interruptFD < 0)
    {
        printf("Unable to get file descriptor for interrupt\n");
        return -1;
    }    
    return interruptFD;
}

/// Show one line of text on the OLED display, using its location, alignment, 
/// size, and color.
void ShowText(I2C_Device* frontPanel, unsigned char align, unsigned char x, unsigned char y, 
                          unsigned char size, int color, std::string text)
{
    unsigned char cmd = align;
    
    int textLen = text.length();
    if(textLen > MAX_OLED_STRING_LEN)
    {
        // truncate text to prevent overrunning the front panel's I2C buffer 
        textLen = MAX_OLED_STRING_LEN;
    }
    
    // the command structure is:
    // [CMD_START][FRAME LENGTH][CMD_OLED][CMD_OLED_xxxTEXT][X BYTE][Y BYTE]
    // [SIZE BYTE] [HI COLOR BYTE][LO COLOR BYTE][TEXT LENGTH][TEXT BYTES] ...
    // [CMD_END]
    unsigned char cmdBuf[35] = 
        {CMD_START, 8 + textLen, CMD_OLED, cmd, x, y, size, 
         (unsigned char)((color & 0xFF00) >> 8), (unsigned char)(color & 0xFF), 
         textLen};
    memcpy(cmdBuf + 10, text.c_str(), textLen);
    cmdBuf[10 + textLen] = CMD_END;
    
    frontPanel->Write(UI_COMMAND, cmdBuf, 11 + textLen);
}

// Clear the screen and show button text (for all but first press).
void HandleButtons(I2C_Device* frontPanel, unsigned char btns)
{
    static bool firstPress = true; 
    
    const unsigned char clear[] = {CMD_START, 2, CMD_OLED, CMD_OLED_CLEAR, CMD_END};
    frontPanel->Write(UI_COMMAND, clear, strlen((const char*)clear));

    if(firstPress)
    {
        // nothing else needed on first detected press
        firstPress = false;
    }
    else
    {
        // display what button event was detected
        std::string text;
        char msg[20];
        switch(btns)
        {
            case BTN1_PRESS:
                text = "Left pressed";
                break;

            case BTN1_HOLD:
                text = "Left held";
                break;

            case BTN2_PRESS:
                text = "Right pressed";
                break;

            case BTN2_HOLD:
                text = "Right held";
                break;

            case BTNS_1_AND_2_PRESS:
                text = "Both pressed";
                break;

            case (BTN1_HOLD | BTN2_HOLD):
                text = "Both held";
                break;

            default:
                sprintf(msg, "error: %X", btns);
                text = msg;
                break;
        }
        ShowText(frontPanel, CMD_OLED_CENTERTEXT, 64, 60, 1, 0xFFFF, text);
    }
}

int main(int argc, char** argv) 
{
    // enable second I2C port 
    system("echo BB-I2C1 > /sys/devices/bone_capemgr.9/slots");

    // use Lanius I2C port by default
    int port = I2C1_PORT;
 
    if(argc > 1) 
    {
        // if there's a command line argument, use Anas I2C port
        port = I2C2_PORT;
    }  
 
    I2C_Device frontPanel(UI_SLAVE_ADDRESS, port);
    int interruptFD = setupPinInput();
    
    // light all LEDs and fade them up and down
    const unsigned char ledSequence[] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 8, CMD_END};
    frontPanel.Write(UI_COMMAND, ledSequence, strlen((const char*)ledSequence));
    
    // show a prompt (note two presses needed because current front panel FW
    // doesn't respond to the first one)
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64,  8, 1, 0xFFFF, "Press button twice");
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 112, 1, 0xFFFF, "to clear the display.");
    
    int pollFd = epoll_create(10);
    if(pollFd < 0)
    {
        printf("Unable to create epoll\n");
        return -1;
    }

    struct epoll_event ev;
    struct epoll_event evs;
    ev.events = EPOLLPRI | EPOLLERR | EPOLLET;
    ev.data.fd = interruptFD;
    if( epoll_ctl(pollFd, EPOLL_CTL_ADD, interruptFD, &ev) != 0) 
    {
        printf("Unable to setup epoll_ctl\n");
        return -1;
    }
    for(;;)
    {
        int status = epoll_wait(pollFd, &evs, 1, -1); // Returns the number of file descriptors ready for the requested io
        if (status) 
        {
            // If we received events in evs, read data
            if ( evs.events & EPOLLPRI )
            {
                // Get the file descriptor of the data that is ready, seek to the beginning
                // and read the data
                char c;
                int fd = evs.data.fd;
                lseek(fd,0,SEEK_SET);
                read(fd, &c, 1);

                // read the button register
                unsigned char btns = frontPanel.Read(BTN_STATUS) & 0xF;
                if(btns != 0) 
                    HandleButtons(&frontPanel, btns);
                else
                {
                    std::cout << "button value was 0" << std::endl;
                }             
            }
        }
    }
    return 0;
}    
    


