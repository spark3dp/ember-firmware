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
    // create the named pipe for reporting status to the webserver
    // "owned" by this network interface
    // don't recreate the FIFO if it exists already
    if (access(STATUS_TO_NET_PIPE, F_OK) == -1) {
        if (mkfifo(STATUS_TO_NET_PIPE, 0666) < 0) {
          Logger::LogError(LOG_ERR, errno, STATUS_TO_NET_PIPE_CREATION_ERROR);
          // we can't really run if we can't update web clients on status
          exit(-1);  
        }
    }
    // Open both ends within this process in non-blocking mode,
    // otherwise open call would wait till other end of pipe
    // is opened by another process
    // but no need to save read fd, since only the web server reads from it
    open(STATUS_TO_NET_PIPE, O_RDONLY|O_NONBLOCK);
    _statusWriteFd = open(STATUS_TO_NET_PIPE, O_WRONLY|O_NONBLOCK);
}

/// Destructor, cleans up pipe used to report status to the Internet
NetworkInterface::~NetworkInterface()
{
    if (access(STATUS_TO_NET_PIPE, F_OK) != -1)
        remove(STATUS_TO_NET_PIPE);
}

/// Handle printer status updates and requests to report that status
void NetworkInterface::Callback(EventType eventType, void* data)
{     
    PrinterStatus* pPS;
    char statusMsg[256];

    switch(eventType)
    {               
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            
            // TODO: here we need to make a local copy of printer status, not just
            // record a reference to the existing one that can be changed
            // perh we want to convert it to JASON here once, and report that whenever requested
            _latestPrinterStatus = *pPS;
            
            if(pPS->_currentLayer != 0 && _statusWriteFd >= 0)  
            {
                // if we're printing, report remaining time
                sprintf(statusMsg, "%d\n", pPS->_estimatedSecondsRemaining);
            }
            else
                sprintf(statusMsg, "%s\n", pPS->_state);
             
            // send status info out the PE status pipe
            lseek(_statusWriteFd, 0, SEEK_SET);
            write(_statusWriteFd, statusMsg, strlen(statusMsg));          
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
