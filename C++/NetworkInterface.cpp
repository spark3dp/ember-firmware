/* 
 * File:   NetworkInterface.cpp
 * Author: Richard Greene
 *
 * Connects the Internet to the EventHandler.
 * 
 * Created on May 14, 2014, 5:45 PM
 */

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include <NetworkInterface.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>

using boost::property_tree::ptree;
using boost::property_tree::ptree_error;

/// Constructor
NetworkInterface::NetworkInterface() :
_statusJSON("\n"),
_statusPushFd(-1)
{
    // see if the named pipe for reporting status to the web
    // has been created
    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1) 
    {
        open(STATUS_TO_WEB_PIPE, O_RDONLY|O_NONBLOCK);
        _statusPushFd = open(STATUS_TO_WEB_PIPE, O_WRONLY|O_NONBLOCK);
        if(_statusPushFd < 0)
            HandleError(StatusToWebPipeOpen);
    }
    
    // TODO: if/when other components need to respond to commands, the 
    // COMMAND_RESPONSE_PIPE should perhaps be created elsewhere
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
            SaveCurrentStatus((PrinterStatus*)data);
            // we only want to push status if there's a listener on
            // the  STATUS_TO_WEB_PIPE.  
            if(_statusPushFd >= 0)
                SendStringToPipe(_statusJSON.c_str(), _statusPushFd);
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    }
}

/// Save the current printer status in JSON.
void NetworkInterface::SaveCurrentStatus(PrinterStatus* pStatus)
{
    try
    {
        ptree pt;
        std::string root = PRINTER_STATUS_KEY ".";
        
        pt.put(root + STATE_PS_KEY, STATE_NAME(pStatus->_state));
        
        const char* change = "none";
        if(pStatus->_change == Entering)
           change = "entering";
        else if(pStatus->_change == Leaving)
           change = "leaving";            
        pt.put(root + CHANGE_PS_KEY, change);
        pt.put(root + IS_ERROR_PS_KEY, pStatus->_isError);        
        pt.put(root + ERROR_CODE_PS_KEY, pStatus->_errorCode); 
        pt.put(root + ERRNO_PS_KEY, pStatus->_errno); 
        pt.put(root + LAYER_PS_KEY, pStatus->_currentLayer);
        pt.put(root + TOAL_LAYERS_PS_KEY, pStatus->_numLayers);
        pt.put(root + SECONDS_LEFT_PS_KEY, pStatus->_estimatedSecondsRemaining);
        pt.put(root + TEMPERATURE_PS_KEY, pStatus->_temperature);
        pt.put(root + UISUBSTATE_PS_KEY, pStatus->_UISubState);
        
        std::stringstream ss;
        write_json(ss, pt);  
        _statusJSON = ss.str();
           
        // remove newlines but add one back at end
        _statusJSON = Replace(_statusJSON, "\n", "") + "\n";
    }
    catch(ptree_error&)
    {
        HandleError(StatusJsonSave);       
    }
}

/// Write the latest printer status to the status to web pipe
void NetworkInterface::SendStringToPipe(const char* str, int fileDescriptor)
{
    if(write(fileDescriptor, str, strlen(str)) != strlen(str))
        HandleError(SendStringToPipeError);
}

/// Handle commands that have already been interpreted
void NetworkInterface::Handle(Command command)
{
 #ifdef DEBUG
//    std::cout << "in NetworkInterface::Handle command = " << 
//                 command << std::endl;
#endif       
    switch(command)
    {
        
        case GetStatus:
            SendStringToPipe(_statusJSON.c_str(), _commandResponseFd);
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
        case StartPauseOrResume:          
        case Test:
        case RefreshSettings:
        case ProcessPrintData:
        case GetSetting:
        case SetSetting:
        case RestoreSetting:
        case GetLogs:
        case SetFirmware:
        case Exit:
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