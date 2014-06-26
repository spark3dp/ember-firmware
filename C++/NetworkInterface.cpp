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
_statusJSON("")
{
    // create the named pipe for reporting status to the web
    // don't recreate the FIFO if it exists already
    if (access(STATUS_TO_WEB_PIPE, F_OK) == -1) {
        if (mkfifo(STATUS_TO_WEB_PIPE, 0666) < 0) {
          LOGGER.LogError(LOG_ERR, errno, STATUS_TO_WEB_PIPE_CREATION_ERROR);
          // we can't really run if we can't update web clients on status
          exit(-1);  
        }
    }
    // Open both ends within this process in non-blocking mode,
    // otherwise open call would wait till other end of pipe
    // is opened by another process
    // no need to save read fd, since only the web reads from it
    open(STATUS_TO_WEB_PIPE, O_RDONLY|O_NONBLOCK);
    _statusWriteFd = open(STATUS_TO_WEB_PIPE, O_WRONLY|O_NONBLOCK);
}

/// Destructor, cleans up pipe used to communicate with webserver
NetworkInterface::~NetworkInterface()
{
    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1)
        remove(STATUS_TO_WEB_PIPE);
}

/// Handle printer status updates and requests to report that status
void NetworkInterface::Callback(EventType eventType, void* data)
{     
    switch(eventType)
    {               
        case PrinterStatusUpdate:
            SaveCurrentStatus((PrinterStatus*)data);
      //      SendCurrentStatus();
            break;
            
        case UICommand:
            // we only handle the GetStatus command
            // TODO: we should be able to use a CommandInterpreter too, no?
            // we should not have to spell out this string here
            if(strcmp(CmdToUpper((char*)data), "GETSTATUS") == 0)
                SendCurrentStatus();
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
        std::string root = "PrinterStatus.";
        
        pt.put(root + "State", pStatus->_state);
        
        const char* change = "none";
        if(pStatus->_change == Entering)
           change = "entering";
        else if(pStatus->_change == Leaving)
           change = "leaving";            
        pt.put(root + "Change", change);
        pt.put(root + "IsError", pStatus->_isError);
        pt.put(root + "ErrorCode", pStatus->_errorCode); 
        pt.put(root + "Error", pStatus->_errorMessage);
        pt.put(root + "Layer", pStatus->_currentLayer);
        pt.put(root + "TotalLayers", pStatus->_numLayers);
        pt.put(root + "SecondsLeft", pStatus->_estimatedSecondsRemaining);
        pt.put(root + "JobName", pStatus->_jobName);
        pt.put(root + "Temperature", pStatus->_temperature);
        
        std::stringstream ss;
        write_json(ss, pt);  
        _statusJSON = ss.str();
           
        // remove newlines but add one back at end
        _statusJSON = Replace(_statusJSON, "\n", "") + "\n";
    }
    catch(ptree_error&)
    {
        LOGGER.LogError(LOG_ERR, errno, STATUS_JSON_SAVE_ERROR);       
    }
}

/// Send the latest printer status to the web
void NetworkInterface::SendCurrentStatus()
{
    try
    {
        // send status info out the web status pipe
        lseek(_statusWriteFd, 0, SEEK_SET);
        write(_statusWriteFd, _statusJSON.c_str(), _statusJSON.length());
    }
    catch(...)
    {
        LOGGER.LogError(LOG_ERR, errno, STATUS_JSON_SEND_ERROR);
    }
}