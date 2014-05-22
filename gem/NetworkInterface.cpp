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

#include <NetworkInterface.h>
#include <Logger.h>
#include <Filenames.h>

/// Constructor
NetworkInterface::NetworkInterface() 
{
    // create the named pipe for reporting status to the web
    // don't recreate the FIFO if it exists already
    if (access(STATUS_TO_WEB_PIPE, F_OK) == -1) {
        if (mkfifo(STATUS_TO_WEB_PIPE, 0666) < 0) {
          Logger::LogError(LOG_ERR, errno, STATUS_TO_WEB_PIPE_CREATION_ERROR);
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

/// Destructor, cleans up pipes used to communicate with webserver
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
            _latestPrinterStatus = (PrinterStatus*)data;
            
            // TODO: here we need to make a local copy of printer status, not just
            // record a reference to the existing one that can be changed
            // perh we want to convert it to JASON here once, and report that whenever requested
                     
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    }
}


/// Send the latest printer status to the web
void NetworkInterface::SendCurrentStatus()
{
    // TODO: here we need to make a local copy of printer status, not just
    // record a reference to the existing one that can be changed
    // perh we want to convert it to JASON here once, and report that whenever requested
    char statusMsg[256];           
                
    if(_latestPrinterStatus->_currentLayer != 0 && _statusWriteFd >= 0)  
    {
        // if we're printing, report remaining time
        sprintf(statusMsg, "%d\n", _latestPrinterStatus->_estimatedSecondsRemaining);
    }
    else
        sprintf(statusMsg, "%s\n", _latestPrinterStatus->_state);

    // send status info out the PE status pipe
    lseek(_statusWriteFd, 0, SEEK_SET);
    write(_statusWriteFd, statusMsg, strlen(statusMsg)); 
}