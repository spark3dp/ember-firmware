//  File:   TerminalUI.cpp
//  Implements the terminal as a UI component, for getting status updates
//  (terminal input is also accepted, via stdin)
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


#include <iostream>

#include <TerminalUI.h>
#include <PrinterStatus.h>
#include <MessageStrings.h>

#include "Logger.h"

void TerminalUI::Callback(EventType eventType, const EventData& data)
{     
    PrinterStatus ps;
    char statusMsg[256];
    const char* change = "";
    const char* substate = "";
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            ps = data.Get<PrinterStatus>();
            if (ps._change == Entering)
                change = "entering ";
            else if (ps._change == Leaving)
                change = "leaving ";
            
            if (ps._UISubState != NoUISubState)
                substate = SUBSTATE_NAME(ps._UISubState);
            
            std::cout <<  change << STATE_NAME(ps._state) << " " << substate;

            if (ps._currentLayer != 0) // if we're printing, show more status 
            {
                sprintf(statusMsg, PRINTER_STATUS_FORMAT, ps._currentLayer, 
                        ps._numLayers,
                        ps._estimatedSecondsRemaining);
                std::cout << statusMsg;             
            }
            std::cout << std::endl;
            break;

        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), 
                                                                    eventType);
            break;
    }
}
