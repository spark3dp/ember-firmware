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
#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if(x);                                            \
  else throw std::exception();  

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <NetworkInterface.h>
#include <Logger.h>
#include <utils.h>
#include <Shared.h>

using namespace rapidjson;

/// Constructor
NetworkInterface::NetworkInterface() :
_statusJSON("\n"),
_statusPushFd(-1)
{    
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

/// Save the current printer status in JSON.
void NetworkInterface::SaveCurrentStatus(PrinterStatus* pStatus)
{
    try
    {
        const char json[] = "{"
            "\"" STATE_PS_KEY "\": \"\","
            "\"" CHANGE_PS_KEY "\": \"\","
            "\"" IS_ERROR_PS_KEY "\": false,"
            "\"" ERROR_CODE_PS_KEY "\": 0,"
            "\"" ERRNO_PS_KEY "\": 0,"
            "\"" LAYER_PS_KEY "\": 0,"
            "\"" TOAL_LAYERS_PS_KEY "\": 0,"
            "\"" SECONDS_LEFT_PS_KEY "\": 0,"
            "\"" TEMPERATURE_PS_KEY "\": 0.0,"
            "\"" UISUBSTATE_PS_KEY "\": 0"
        "}"; 
 
        Document doc;
        doc.Parse(json);
        
        Value s;
        const char* str = STATE_NAME(pStatus->_state);
        s.SetString(str, strlen(str), doc.GetAllocator());       
        doc[STATE_PS_KEY] = s; 
        
        s = NO_CHANGE;
        if(pStatus->_change == Entering)
           s = ENTERING;
        // should be impossible case, since we've filtered out cases of Leaving 
        else if(pStatus->_change == Leaving)
           s = LEAVING;
        doc[CHANGE_PS_KEY] = s; 
        
        doc[IS_ERROR_PS_KEY] = pStatus->_isError;        
        doc[ERROR_CODE_PS_KEY] = pStatus->_errorCode; 
        doc[ERRNO_PS_KEY] = pStatus->_errno; 
        doc[LAYER_PS_KEY] = pStatus->_currentLayer;
        doc[TOAL_LAYERS_PS_KEY] = pStatus->_numLayers;
        doc[SECONDS_LEFT_PS_KEY] = pStatus->_estimatedSecondsRemaining;
        doc[TEMPERATURE_PS_KEY] = pStatus->_temperature;
        
        str = SUBSTATE_NAME(pStatus->_UISubState);
        s.SetString(str, strlen(str), doc.GetAllocator()); 
        doc[UISUBSTATE_PS_KEY] = s;
        
        StringBuffer buffer; 
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);        
        _statusJSON = buffer.GetString();
        _statusJSON += "\n";
    }
    catch(std::exception)
    {
        HandleError(StatusJsonSave);       
    }
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