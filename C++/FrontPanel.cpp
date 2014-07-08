/* 
 * File:   FrontPanel.cpp
 * Author: Richard Greene
 * 
 * Implements the front panel UI device, with its buttons and displays
 * 
 * Created on April 15, 2014, 12:43 PM
 */

#include <iostream>  // for debug only

#include <FrontPanel.h>
#include <Hardware.h>

/// Public constructor, base class opens I2C connection and sets slave address
FrontPanel::FrontPanel(unsigned char slaveAddress) :
I2C_Device(slaveAddress)
{
    // turn on the OLED display and clear it
    unsigned char cmdBuf[6] = {CMD_START, 4, CMD_OLED, CMD_OLED_ON, CMD_OLED, CMD_OLED_CLEAR};
    Write(UI_COMMAND, cmdBuf, 6);
}

/// Base class closes connection to the device
FrontPanel::~FrontPanel() 
{
}    

/// Handles events forwarded by the event handler
void FrontPanel::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case PrinterStatusUpdate:
            ShowStatus((PrinterStatus*)data); 
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}

/// Updates the front panel displays, based on printer status
void FrontPanel::ShowStatus(PrinterStatus* pPS)
{
    // TODO: replace placeholder code below
    
 //   ShowText(63, 63, 1, 0xA55A, pPS->_state);
    
    static int n = 0;
    if(pPS->_currentLayer != 0)
    {
        // we're currently printing, so
        // update the bar graph with % completed
        double pctComplete = (pPS->_currentLayer - 1) * 100.0 / pPS->_numLayers;
        
        // and update the OLED display with % completed and remaining time
        int hrs = pPS->_estimatedSecondsRemaining / 3600;
        int min = (pPS->_estimatedSecondsRemaining - (hrs * 3600)) / 60;
        int sec = pPS->_estimatedSecondsRemaining - (hrs * 3600) - min * 60;
        
        // for now, just print that to stdout
        std::cout << "      front Panel displays " << pctComplete << "%  " << 
                     hrs << ":" << min << ":" << sec << std::endl;
    }
    else
    {
        // based on pPS->_state, update the OLED display,
        
        // and perhaps update the bar graph or run an animation on it
        // for now, just:
        if(++n > 8)
            n = 1;
    //    AnimateLEDRing(n);
    }
}

/// Show an LED ring animation.
void FrontPanel::AnimateLEDRing(unsigned char n)
{
#ifdef DEBUG
//    std::cout << "LED animation #" << (int)n << std::endl;
#endif

    // TODO: if n = 0 is a legitimate value, we'll need to change the low-level 
    // Write (char*) command to not just use strlen!
    unsigned char cmdBuf[5] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, n};
    Write(UI_COMMAND, cmdBuf, 5);
}



// Display a line of text on the OLED display.
void FrontPanel::ShowText(unsigned char x, unsigned char y, unsigned char size, 
                          int color, const char* text)
{
#ifdef DEBUG
//    std::cout << "Showing text: " << text << std::endl;
#endif
    int textLen = strlen(text);
    if(textLen > 25)
        textLen = 25;
    // [CMD_OLED][CMD_OLED_SETTEXT][X BYTE][Y BYTE][SIZE BYTE][HI COLOR BYTE][LO COLOR BYTE][TEXT LENGTH BYTE][TXT BYTES]
    unsigned char cmdBuf[35] = 
        {CMD_START, 8 + textLen, CMD_OLED, CMD_OLED_SETTEXT, x, y, size, 
         ((unsigned char)(color & 0xFF00)) >> 8, (unsigned char)(color & 0xFF), 
         textLen};
    memcpy(cmdBuf + 10, text, textLen);
    Write(UI_COMMAND, cmdBuf, 10 + textLen);
}