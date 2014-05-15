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
#include <PrinterStatus.h>
#include <Logger.h>

NetworkInterface::NetworkInterface()
{
    // create the named pipe for reporting status to the webserver
    // "owned" by this network interface
    char pipeName[] = "/tmp/StatusToNetPipe";
    // don't recreate the FIFO if it exists already
    if (access(pipeName, F_OK) == -1) {
        if (mkfifo(pipeName, 0666) < 0) {
          Logger::LogError(LOG_ERR, errno, STATUS_TO_NET_PIPE_CREATION_ERROR);
          // we can't really run if we can't update web clients on status
          exit(-1);  
        }
    }
    // Open both ends within this process in non-blocking mode,
    // otherwise open call would wait till other end of pipe
    // is opened by another process
    // but no need to save read fd, since only the web server reads from it
    open(pipeName, O_RDONLY|O_NONBLOCK);
    _statusWriteFd = open(pipeName, O_WRONLY|O_NONBLOCK);
}

void NetworkInterface::Callback(EventType eventType, void* data)
{     
    PrinterStatus* pPS;
    char statusMsg[256];

    switch(eventType)
    {               
        case PrinterStatusUpdate:
            pPS = (PrinterStatus*)data;
            if(pPS->_currentLayer != 0 && _statusWriteFd >= 0)  
            {
                // if we're printing, report remaining time
                sprintf(statusMsg, "%d\n", pPS->_estimatedSecondsRemaining);
             
                // send status info out the PE status pipe
                lseek(_statusWriteFd, 0, SEEK_SET);
                write(_statusWriteFd, statusMsg, strlen(statusMsg));          
            }
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
