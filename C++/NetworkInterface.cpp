//  File:   NetworkInterface.cpp
//  Serializes printer status to networks
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

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

#include <NetworkInterface.h>
#include <Logger.h>
#include <utils.h>
#include <Shared.h>
#include <Settings.h>

// Constructor
NetworkInterface::NetworkInterface() :
_statusJSON("\n"),
_statusPushFd(-1)
{    
}

// Destructor
NetworkInterface::~NetworkInterface()
{

}

// Handle printer status updates and requests to report that status
void NetworkInterface::Callback(EventType eventType, const EventData& data)
{     
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            // we don't care about states that are being left
            if (data.Get<PrinterStatus>()._change != Leaving)
            {
                SaveCurrentStatus(data.Get<PrinterStatus>());
                // we only want to push status if there's a listener on
                // the pipe used for reporting status to the web 
                if (_statusPushFd < 0)
                {
                    // see if that pipe has been created
                    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1) 
                    {
                        open(STATUS_TO_WEB_PIPE, O_RDONLY|O_NONBLOCK);
                        _statusPushFd = open(STATUS_TO_WEB_PIPE, 
                                             O_WRONLY|O_NONBLOCK);
                        if (_statusPushFd < 0)
                            LOGGER.HandleError(StatusToWebPipeOpen);
                    }
                }
                if (_statusPushFd >= 0)
                    SendStringToPipe(_statusJSON.c_str(), _statusPushFd);
            }
            break;
            
        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), 
                                                                    eventType);
            break;
    }
}

// Save the current printer status in a JSON string and a file.
void NetworkInterface::SaveCurrentStatus(const PrinterStatus& status)
{
    _statusJSON = status.ToString();
    
    // save it in a file as well
    std::ofstream statusFile(PRINTER_STATUS_FILE, std::ios::trunc);
    if (!statusFile.is_open())
    {
        LOGGER.HandleError(SaveStatusToFileError);
        return;
    }
    statusFile << _statusJSON;
}

// Write the latest printer status to the status to web pipe
void NetworkInterface::SendStringToPipe(std::string str, int fileDescriptor)
{
    if (write(fileDescriptor, str.c_str(), str.length()) != str.length())
        LOGGER.HandleError(SendStringToPipeError);
}
 