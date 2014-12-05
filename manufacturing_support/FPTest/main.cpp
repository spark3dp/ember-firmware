/* 
 * File:   main.cpp
 * 
 * For testing Ember front panel during manufacturing.
 * 
 * Author: Richard Greene
 *
 * Created on December 5, 2014, 9:39 AM
 */

#include <I2C_Device.h>

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

char GPIOInputValue[64];
FILE *inputHandle = NULL;

// set up front panel interrupt pin as an input
void setupPinInput()
{
    char setValue[4], GPIOInputString[4], GPIODirection[64];
    // setup input
    sprintf(GPIOInputString, "%d", UI_INTERRUPT_PIN);
    sprintf(GPIOInputValue, "/sys/class/gpio/gpio%d/value", UI_INTERRUPT_PIN);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", UI_INTERRUPT_PIN);
 
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

int main(int argc, char** argv) {

    int port = I2C2_PORT;
    
// TODO, use cmd line argument to determine I2C port to use    
#ifdef LANIUS    
    // enable second I2C port 
    system("echo BB-I2C1 > /sys/devices/bone_capemgr.9/slots");
    
    port = I2C1_PORT;
#endif    
 
    I2C_Device frontPanel(UI_SLAVE_ADDRESS, port);
    setupPinInput();
    
    // light all LEDs and fade them up and down
    const unsigned char ledSequence[] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 8, CMD_END};
    frontPanel.Write(UI_COMMAND, ledSequence, strlen((const char*)ledSequence));
    
    // show a prompt (note two presses needed because current front panel FW
    // doesn't respond to the first one)
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64,  8, 1, 0xFFFF, "Press button twice");
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 112, 1, 0xFFFF, "to clear the display.");
    
    bool firstPress = true;
    for(;;)
    {
        // await button event, signaled by interrupt pin going high
        char interrupt = '0';
        while(interrupt != '1')
        {
            if ((inputHandle = fopen(GPIOInputValue, "rb+")) == NULL)
            {
                printf("Unable to open input handle\n");
                exit (EXIT_FAILURE) ;
            }
            fread(&interrupt, sizeof(char), 1, inputHandle);
            fclose(inputHandle);  

            usleep(100000);
        } 
        
        unsigned char btns = frontPanel.Read(BTN_STATUS) & 0xF;
        
        // button event detected, so clear screen
        const unsigned char clear[] = {CMD_START, 2, CMD_OLED, CMD_OLED_CLEAR, CMD_END};
        frontPanel.Write(UI_COMMAND, clear, strlen((const char*)clear));
        
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
            ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 60, 1, 0xFFFF, text);
   //         sleep(1);
        }  
    }
    return 0;
}

