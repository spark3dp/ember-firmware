
/* 
 * File:   TerminalUI.cpp
 * Author: Richard Greene
 * 
 * Implements the terminal as a UI component, for getting status updates.
 * (Terminal input is also accepted, via stdin in EventHandler.)
 *
 * Created on April 18, 2014, 4:54 PM
 */

#include <iostream>

#include <TerminalUI.h>
#include <PrinterStatus.h>

void TerminalUI::Callback(EventType eventType, void* data)
{     
    PrinterStatus* pPS;
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            std::cout <<  pPS->_state;

            if(pPS->_currentLayer != 0) // if we're printing, show additional status 
            {
                std::cout <<", layer " << 
                            pPS->_currentLayer <<
                            ", seconds left: " << 
                            pPS->_estimatedSecondsRemaining;             
            }
            std::cout << std::endl;
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
