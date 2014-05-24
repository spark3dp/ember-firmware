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

#include <NetworkInterface.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>

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
            SaveCurrentStatus((PrinterStatus*)data);
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
    FILE * pFile;
    pFile = fopen (LATEST_STATUS_JSON,"w+");
    if (pFile!=NULL)
    {
        fprintf(pFile, "{\n\t\"State\": \"%s\",\n", pStatus->_state);
        const char* change = "none";
        if(pStatus->_change == Entering)
           change = "entering";
        else if(pStatus->_change == Leaving)
           change = "leaving";    
        fprintf(pFile, "\t\"Change\": \"%s\",\n", change);
        fprintf(pFile, "\t\"IsError\": \"%s\",\n", pStatus->_isError ? 
                                                             "true" : "false");
        fprintf(pFile, "\t\"ErrorCode\": %d,\n", pStatus->_errorCode);
        fprintf(pFile, "\t\"Error\": \"%s\",\n", pStatus->_errorMessage);
        fprintf(pFile, "\t\"Layer\": %d,\n", pStatus->_currentLayer);
        fprintf(pFile, "\t\"TotalLayers\": %d,\n", pStatus->_numLayers);
        fprintf(pFile, "\t\"SecondsLeft\": %d,\n", pStatus->_estimatedSecondsRemaining);
        fprintf(pFile, "\t\"JobName\": \"%s\",\n", pStatus->_jobName);
        fprintf(pFile, "\t\"Temperature\": %.2f\n}\n", pStatus->_temperature);

        fclose (pFile);
    }
    else
        Logger::LogError(LOG_ERR, errno, STATUS_JSON_OPEN_ERROR);
}

/// Send the latest printer status to the web
void NetworkInterface::SendCurrentStatus()
{
    // send status info out the PE status pipe
    lseek(_statusWriteFd, 0, SEEK_SET);
    char buf[256];
    FILE * pFile;
    pFile = fopen (LATEST_STATUS_JSON,"r");
    if (pFile!=NULL)
    {
        while(fgets(buf, sizeof(buf), pFile) != NULL)
            write(_statusWriteFd, buf, strlen(buf));
        
        fclose (pFile);
    }
    else
        Logger::LogError(LOG_ERR, errno, STATUS_JSON_OPEN_ERROR);    
}