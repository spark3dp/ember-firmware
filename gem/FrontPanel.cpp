/* 
 * File:   FrontPanel.cpp
 * Author: Richard Greene
 * 
 * Implements the front panel UI device, with its buttons and displays
 * 
 * Created on April 15, 2014, 12:43 PM
 */

#include <FrontPanel.h>
#include <iostream>  // for debug only

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
}