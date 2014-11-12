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
    // create the named pipe for reporting command responses to the web
    // don't recreate the FIFO if it exists already
    if (access(COMMAND_RESPONSE_PIPE, F_OK) == -1) {
        if (mkfifo(COMMAND_RESPONSE_PIPE, 0666) < 0) {
          HandleError(CommandResponsePipeCreation);
          // we can't really run if we can't respond to commands
          exit(-1);  
        }
    }
    // no need to save read fd, since only the web reads from it
    open(COMMAND_RESPONSE_PIPE, O_RDONLY|O_NONBLOCK);
    _commandResponseFd = open(COMMAND_RESPONSE_PIPE, O_WRONLY|O_NONBLOCK);

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
                            HandleError(StatusToWebPipeOpen);
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

/// Save the current printer status in a JSON string.
void NetworkInterface::SaveCurrentStatus(PrinterStatus* pStatus)
{
    _statusJSON = pStatus->ToString();
}

/// Write the latest printer status to the status to web pipe
void NetworkInterface::SendStringToPipe(std::string str, int fileDescriptor)
{
    if(write(fileDescriptor, str.c_str(), str.length()) != str.length())
        HandleError(SendStringToPipeError);
}

/// Handle commands that have already been interpreted
void NetworkInterface::Handle(Command command)
{
 #ifdef DEBUG
//    std::cout << "in NetworkInterface::Handle command = " << 
//                command << std::endl;
#endif       
    switch(command)
    { 
        case GetStatus:
            SendStringToPipe(_statusJSON, _commandResponseFd);
            break;
       
        case GetFWVersion:
            SendStringToPipe(GetFirmwareVersion(), _commandResponseFd);
            break;
            
        case GetBoardNum:
            SendStringToPipe(GetBoardSerialNum(), _commandResponseFd);
            break;
            
        // none of these commands are handled by the network interface
        // (or at least not yet in some cases)
        case Start:                 
        case Cancel:
        case Pause:
        case Resume:
        case Reset:           
        case Test:
        case RefreshSettings:
        case StartPrintDataLoad:
        case ProcessPrintData:
        case ShowPrintDataLoaded:
        case GetSetting:
        case SetSetting:
        case RestoreSetting:
        case ApplyPrintSettings:
        case GetLogs:
        case SetFirmware:
        case Exit:
        case StartRegistering:
        case RegistrationSucceeded:     
        case StartCalibration:
            break;

        default:
            HandleError(UnknownCommandInput, false, NULL, command); 
            break;
    }
}
  
/// Handles errors by simply logging them
void NetworkInterface::HandleError(ErrorCode code, bool fatal, 
                                   const char* str, int value)
{
    LOGGER.HandleError(code, fatal, str, value);
}