/* 
 * File:   NetworkIFUT.cpp
 * Author: Richard Greene
 *
 * Created on May 28, 2014, 1:31:54 PM
 */

#include <stdlib.h>
#include <iostream>
#include <sys/stat.h>

/*
 * Simple C++ Test Suite
 */
#include <NetworkInterface.h>
#include <Filenames.h>
#include <Shared.h>
#include <CommandInterpreter.h>

FILE* _pPushedStatusPipe;
FILE* _pCmdResponsePipe;

bool ExpectedStatus(const char* state, const char* temp, FILE* file)
{
    // std::cout << "looking for " << state << " " << temp << std::endl;
    char buf[1024];
    bool foundState = false;
    bool foundTemp = false;
    
    fgets(buf, 1024, file);
    // std::cout << buf << std::endl;
    if (strstr(buf, "State") != NULL &&  strstr(buf, state) != NULL) 
    {
        foundState = true;
    }
    if(strstr(buf, "Temperature") != NULL && strstr(buf, temp) != NULL)
    {
        foundTemp = true;
    }
 
    return foundState  && foundTemp;
}


void test1() {
    std::cout << "NetworkIFUT test 1" << std::endl;
    
    char buf[256];
    
    // delete the named pipe used for Web status, if it exists
    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1)
        remove(STATUS_TO_WEB_PIPE);
        
    // delete the named pipe used for responding to commands, if it exists
    if (access(COMMAND_RESPONSE_PIPE, F_OK) != -1)
        remove(COMMAND_RESPONSE_PIPE);
        
    // create the named pipe used for Web status _before_ constructing 
    // the NetworkINterface
    mkfifo(STATUS_TO_WEB_PIPE, 0666);

    NetworkInterface net;
    CommandInterpreter cmdInterp(&net);
    
    // open the named pipes used for pushed status, commands, & responses
    _pPushedStatusPipe = fopen (STATUS_TO_WEB_PIPE, "r+");
    _pCmdResponsePipe = fopen (COMMAND_RESPONSE_PIPE, "r+");    
    
    // set some printer status
    PrinterStatus ps;
    ps._state  = PrintingLayerState;
    ps._temperature = 3.14159;

    // send it in an update event to a NetworkInterface
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, &ps);
    
    // check the automatically reported status
    if(!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pPushedStatusPipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature" << std::endl;
    
    // report that status to the net
    strcpy(buf, "GetStatus\n");
    ((ICallback*)&cmdInterp)->Callback(UICommand, buf);
    
    if(!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature again" << std::endl;
     
    ps._state  = EndingPrintState;
    ps._temperature = 42;
    
    // check status again (should not have changed)
    ((ICallback*)&cmdInterp)->Callback(UICommand, buf);
    
    if(!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find unchanged printer state and temperature" << std::endl;
    
    // send an update event with the new status
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, &ps);
    
    // check the automatically reported status
    if(!ExpectedStatus(STATE_NAME(EndingPrintState), "42", _pPushedStatusPipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature" << std::endl;    
    
    ((ICallback*)&cmdInterp)->Callback(UICommand, buf);
    
    if(!ExpectedStatus(STATE_NAME(EndingPrintState), "42", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature again" << std::endl;
    
    // check the firmware version command
    strcpy(buf, "GetFWversion\n");
    ((ICallback*)&cmdInterp)->Callback(UICommand, buf);
    
    char fbuf[256];   
    fgets(fbuf, 256, _pCmdResponsePipe);
    if(strcmp(fbuf, FIRMWARE_VERSION "\n") != 0)
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=Expected to get firmware version " << FIRMWARE_VERSION << " but got " << fbuf << std::endl;
    else
        std::cout << "Found expected firmware version: " << fbuf << std::endl;
    
    // check the board serial number command
    strcpy(buf, "getBoardNum\n");
    ((ICallback*)&cmdInterp)->Callback(UICommand, buf);
       
    fgets(fbuf, 256, _pCmdResponsePipe);
    char* t1 = strstr(fbuf, "BBBK");
    if(strstr(fbuf, "BBBK") == NULL)
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to get board serial number: " << fbuf << std::endl;
    else
        std::cout << "Found expected board serial number: " << fbuf << std::endl;    
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% NetworkIFUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (NetworkIFUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (NetworkIFUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1)
        remove(STATUS_TO_WEB_PIPE);
    
     if (access(COMMAND_RESPONSE_PIPE, F_OK) != -1)
        remove(COMMAND_RESPONSE_PIPE);
    
    return (EXIT_SUCCESS);
}
