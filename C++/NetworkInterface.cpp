/* 
 * File:   NetworkInterface.cpp
 * Author: Richard Greene
 *
 * Sends printer info to named pipes for the web client to send to the web.
 * 
 * Created on May 14, 2014, 5:45 PM
 */

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

/// Constructor
NetworkInterface::NetworkInterface() :
_statusJSON("\n"),
_statusPushFd(-1)
{    
}

/// Destructor
NetworkInterface::~NetworkInterface()
{

}

/// Handle printer status updates and requests to report that status
void NetworkInterface::Callback(EventType eventType, void* data)
{     
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            // we don't care about states that are being left
            if(((PrinterStatus*)data)->_change != Leaving)
            {
                SaveCurrentStatus((PrinterStatus*)data);
                // we only want to push status if there's a listener on
                // the pipe used for reporting status to the web 
                if(_statusPushFd < 0)
                {
                    // see if that pipe has been created
                    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1) 
                    {
                        open(STATUS_TO_WEB_PIPE, O_RDONLY|O_NONBLOCK);
                        _statusPushFd = open(STATUS_TO_WEB_PIPE, O_WRONLY|O_NONBLOCK);
                        if(_statusPushFd < 0)
                            LOGGER.HandleError(StatusToWebPipeOpen);
                    }
                }
                if(_statusPushFd >= 0)
                    SendStringToPipe(_statusJSON.c_str(), _statusPushFd);
            }
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    }
}

/// Save the current printer status in a JSON string and a file.
void NetworkInterface::SaveCurrentStatus(PrinterStatus* pStatus)
{
    _statusJSON = pStatus->ToString();
    
    // save it in a file as well
    std::ofstream statusFile(PRINTER_STATUS_FILE, std::ios::trunc);
    if (!statusFile.is_open())
    {
        LOGGER.HandleError(SaveStatusToFileError);
        return;
    }
    statusFile << _statusJSON;
}

/// Write the latest printer status to the status to web pipe
void NetworkInterface::SendStringToPipe(std::string str, int fileDescriptor)
{
    if(write(fileDescriptor, str.c_str(), str.length()) != str.length())
        LOGGER.HandleError(SendStringToPipeError);
}
 