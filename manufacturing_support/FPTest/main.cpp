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
#include <string.h>
#include <string>

using namespace std;

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
#ifdef LANIUS    
    // enable second I2C port 
    system("echo BB-I2C1 > /sys/devices/bone_capemgr.9/slots");
    
    port = I2C1_PORT;
#endif    
 
    I2C_Device frontPanel(UI_SLAVE_ADDRESS, port);
    
    // light all LEDs and fade them up and down
    const unsigned char ledSequence[] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 8, CMD_END};
    frontPanel.Write(UI_COMMAND, ledSequence, strlen((const char*)ledSequence));
    
    // show a prompt
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64,  8, 1, 0xFFFF, "Press button twice");
    ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 112, 1, 0xFFFF, "to clear the display.");
   
    return 0;
}

