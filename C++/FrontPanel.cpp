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
    // all TODO
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
    }
    
    static int n = 0;
    if(++n > 10)
        n = 0;
    AnimateLEDRing(n);
}

#define CMD_START 0x98
#define CMD_RING 0x01 //!< Put in ring command mode
#define CMD_RING_SEQUENCE 0x02 //!< Start a ring sequence (0 to stop)

// Show an LED ring animation.
void FrontPanel::AnimateLEDRing(unsigned char n)
{
#ifdef DEBUG
 //    std::cout << "LED animation #" << (int)n << std::endl;
#endif

    // TODO: if n = 0 is a legitimate value, we'll need to change the low-level 
    // Write (char*) command to not just use strlen!
    unsigned char cmdBuf[6] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, n, 0};
 //   Write(UI_COMMAND, cmdBuf);
}