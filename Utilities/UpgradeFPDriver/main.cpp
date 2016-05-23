//  File:   main.cpp
//  Displays status messages on front panel during kernel upgrade
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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

#include <string>
#include <cstring>
#include <iostream>

#include "I2C_Device.h"
#include "Hardware.h"

// Show one line of text on the OLED display, using its location, alignment, 
// size, and color.
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
    unsigned char cmdBuf[35] = {
        CMD_START,
        static_cast<unsigned char>(8 + textLen),
        CMD_OLED,
        cmd,
        x,
        y,
        size,
        static_cast<unsigned char>((color & 0xFF00) >> 8),
        static_cast<unsigned char>(color & 0xFF), 
        static_cast<unsigned char>(textLen)
    };

    std::memcpy(cmdBuf + 10, text.c_str(), textLen);
    cmdBuf[10 + textLen] = CMD_END;
    
    frontPanel->Write(FP_COMMAND, cmdBuf, 11 + textLen);
}

void ShowUsage()
{
    std::cerr << "must specify \"begin\" or \"complete\" as only argument"
            << std::endl;

}

int main(int argc, char** argv) 
{
    if (argc != 2)
    {
        ShowUsage();
        return 1;
    }

    std::string mode(argv[1]);

    if (!(mode == "begin" || mode =="complete"))
    {
        ShowUsage();
        return 1;
    }
    
    // use Lanius I2C port by default
    int port = I2C1_PORT;
 
    I2C_Device frontPanel(FP_SLAVE_ADDRESS, port);

    const unsigned char clear[] = {
        CMD_START,
        2,
        CMD_OLED,
        CMD_OLED_CLEAR,
        CMD_END
    };
    frontPanel.Write(FP_COMMAND, clear, sizeof(clear));

    if (mode == "begin")
    {
        // light all LEDs and fade them up and down
        const unsigned char ledSequence[] = {
            CMD_START,
            3,
            CMD_RING,
            CMD_RING_SEQUENCE,
            2,
            CMD_END
        };
        frontPanel.Write(FP_COMMAND, ledSequence, sizeof(ledSequence));
        
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 36, 1, 0xFFFF, "Upgrade in");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 56, 1, 0xFFFF, "progress...");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 76, 1, 0xFFFF, "Please wait");
    }
    else
    {
        // light all LEDs and fade them up and down
        const unsigned char ledSequence[] = {
            CMD_START,
            3,
            CMD_RING,
            CMD_RING_SEQUENCE,
            8,
            CMD_END
        };
        frontPanel.Write(FP_COMMAND, ledSequence, sizeof(ledSequence));
        
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 16, 1, 0xFFFF, "Upgrade complete");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 36, 1, 0xFFFF, "Please disconnect");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 56, 1, 0xFFFF, "power, remove SD");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 76, 1, 0xFFFF, "card, and reconnect");
        ShowText(&frontPanel, CMD_OLED_CENTERTEXT, 64, 96, 1, 0xFFFF, "power");
    }
    
    return 0;
}    
    


