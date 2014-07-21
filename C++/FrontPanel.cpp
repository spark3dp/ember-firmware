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
    // turn on the OLED display 
    unsigned char cmdBuf[4] = {CMD_START, 2, CMD_OLED, CMD_OLED_ON};
    Write(UI_COMMAND, cmdBuf, 4);
    
    ClearScreen();
    
    // stop any animation in  progress and turn off all LEDs
    ShowLEDGraph(0);
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
        
        
        if(pPS->_change == Entering)
        {   
            if(strcmp(pPS->_state, "Exposing") == 0)
            {
                ClearScreen();
                char pctMsg[20];
                sprintf(pctMsg,"%d:%02d", hrs, min);
                ShowText(10, 50, 2, 0xFFFF, pctMsg);
                
                ShowLEDGraph((int) (pctComplete * 21.0 / 100.0 + 0.5));
            }
            else if(strcmp(pPS->_state, "Separating") != 0)
            {
                ClearScreen();
                ShowText(1, 30, 1, 0xFFFF, pPS->_state);
            }
        }
    }
    else
    {
        // based on pPS->_state, update the OLED display
        if(pPS->_change == Entering)
        {
            ClearScreen();
            ShowText(1, 30, 1, 0xFFFF, pPS->_state);
            
            // test LED ring animations:
            if(strcmp(pPS->_state, "Home") == 0)
            {
//                // try turning off previous animation first
//                AnimateLEDRing(0);
                
                if(++n > 7)
                    n = 1;
                
                AnimateLEDRing(n);
            }
        }
    }
}

/// Illuminate the given number of LEDs.
void FrontPanel::ShowLEDGraph(int numLeds)
{
    // first stop any animation in  progress
    AnimateLEDRing(0);
    
#ifdef DEBUG
 //   std::cout << "About to show " << numLeds << " LEDs" << std::endl;
#endif    
    
    // and turn all the LEDs off
    unsigned char cmdBuf[6] = {CMD_START, 4, CMD_RING, CMD_RING_LEDS, 0, 0};
    Write(UI_COMMAND, cmdBuf, 6);
    
    if(numLeds == 0)
        return;
    
    // turn on individual LEDs
    for(unsigned char i = 0; i < numLeds; i++)
    {
        // show them at full intensity
        unsigned char cmdBuf2[7] = {CMD_START, 5, CMD_RING, CMD_RING_LED, i, 0xFF, 0xFF};
        Write(UI_COMMAND, cmdBuf2, 7);
    }
}

/// Show an LED ring animation.
void FrontPanel::AnimateLEDRing(unsigned char n)
{
#ifdef DEBUG
    std::cout << "LED animation #" << (int)n << std::endl;
#endif
    
    unsigned char cmdBuf[5] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, n};
    Write(UI_COMMAND, cmdBuf, 5);
}

void FrontPanel::ClearScreen()
{
    unsigned char cmdBuf[4] = {CMD_START, 2, CMD_OLED, CMD_OLED_CLEAR};
    Write(UI_COMMAND, cmdBuf, 4);
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
         (unsigned char)((color & 0xFF00) >> 8), (unsigned char)(color & 0xFF), 
         textLen};
    memcpy(cmdBuf + 10, text, textLen);
    Write(UI_COMMAND, cmdBuf, 10 + textLen);
}