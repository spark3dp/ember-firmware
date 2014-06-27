/* 
 * File:   NetworkIFUT.cpp
 * Author: Richard Greene
 *
 * Created on May 28, 2014, 1:31:54 PM
 */

#include <stdlib.h>
#include <iostream>

/*
 * Simple C++ Test Suite
 */
#include <NetworkInterface.h>
#include <Filenames.h>

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
        
    NetworkInterface net;
    
    // open the named pipes used for pushed status and command responses
    _pPushedStatusPipe = fopen (STATUS_TO_WEB_PIPE, "r+");
    _pCmdResponsePipe = fopen (COMMAND_RESPONSE_PIPE, "r+");
    
    // set some printer status
    PrinterStatus ps;
    ps._state  = "MyOwnVerySpecialStatus";
    ps._temperature = 3.14159;

    // send it in an update event to a NetworkInterface
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, &ps);
    
    // check the automatically reported status
    if(!ExpectedStatus("MyOwnVerySpecialStatus", "3.14159", _pPushedStatusPipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature" << std::endl;
    
    // report that status to the net
    strcpy(buf, "GetStatus\n");
    ((ICallback*)&net)->Callback(UICommand, buf);
    
    if(!ExpectedStatus("MyOwnVerySpecialStatus", "3.14159", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature again" << std::endl;
     
    ps._state  = "SomeOtherStatus";
    ps._temperature = 42;
    
    // check status again (should not have changed)
    ((ICallback*)&net)->Callback(UICommand, buf);
    
    if(!ExpectedStatus("MyOwnVerySpecialStatus", "3.14159", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find unchanged printer state and temperature" << std::endl;
    
    // send an update event with the new status
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, &ps);
    
    // check the automatically reported status
    if(!ExpectedStatus("SomeOtherStatus", "42", _pPushedStatusPipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature" << std::endl;    
    
    ((ICallback*)&net)->Callback(UICommand, buf);
    
    if(!ExpectedStatus("SomeOtherStatus", "42", _pCmdResponsePipe))
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature again" << std::endl;
    else
        std::cout << "%TEST_PASSED% time=0 testname=test1 (NetworkIFUT) message=found expected printer status" << std::endl; 
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
