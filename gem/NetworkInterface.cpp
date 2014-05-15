/* 
 * File:   NetworkInterface.cpp
 * Author: Richard Greene
 *
 * Connects the Internet to the EventHandler.
 * 
 * Created on May 14, 2014, 5:45 PM
 */

#include <iostream>

#include <NetworkInterface.h>
#include <PrinterStatus.h>

void NetworkInterface::Callback(EventType eventType, void* data)
{     
    PrinterStatus* pPS;
    char statusMsg[256];
    const char* change = "";
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            if(pPS->_change == Entering)
                change = "entering ";
            else if(pPS->_change == Leaving)
                change = "leaving ";
            
            // TODO: instead of sending parsed status to cout, we'll send it to a named pipe
//            std::cout <<  change << pPS->_state;
//
//            if(pPS->_currentLayer != 0) // if we're printing, show additional status 
//            {
//                sprintf(statusMsg, PRINTER_STATUS_FORMAT, pPS->_currentLayer, 
//                        pPS->_numLayers,
//                        pPS->_estimatedSecondsRemaining);
//                std::cout << statusMsg;             
//            }
//            std::cout << std::endl;
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
