//  File:   NetworkIFUT
//  Tests NetworkInterface
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <iostream>
#include <sys/stat.h>

#include <NetworkInterface.h>
#include <Filenames.h>
#include <Shared.h>
#include <CommandInterpreter.h>

int mainReturnValue = EXIT_SUCCESS;

FILE* _pPushedStatusPipe;
FILE* _pPrinterStatusFile;

bool ExpectedStatus(const char* state, const char* temp, FILE* file)
{
    // std::cout << "looking for " << state << " " << temp << std::endl;
    char buf[1024];
    bool foundState = false;
    bool foundTemp = false;
    
    fgets(buf, 1024, file);
    // std::cout << buf << std::endl;
    if (strstr(buf, STATE_PS_KEY) != NULL &&  strstr(buf, state) != NULL) 
    {
        foundState = true;
    }
    if (strstr(buf, TEMPERATURE_PS_KEY) != NULL && strstr(buf, temp) != NULL)
    {
        foundTemp = true;
    }
 
    return foundState  && foundTemp;
}


void test1() {
    std::cout << "NetworkIFUT test 1" << std::endl;
    
    // delete the named pipe used for Web status, if it exists
    if (access(STATUS_TO_WEB_PIPE, F_OK) != -1)
        remove(STATUS_TO_WEB_PIPE);
        
    // create the named pipe used for Web status _before_ constructing 
    // the NetworkINterface
    mkfifo(STATUS_TO_WEB_PIPE, 0666);

    NetworkInterface net;
    
    // open the named pipes used for pushed status 
    _pPushedStatusPipe = fopen(STATUS_TO_WEB_PIPE, "r+");
    // open the file used to pull printer status
    _pPrinterStatusFile = fopen(PRINTER_STATUS_FILE, "w+");
    
    // set some printer status
    PrinterStatus ps;
    ps._state  = PrintingLayerState;
    ps._temperature = 3.14159;

    // send it in an update event to a NetworkInterface
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, EventData(ps));
    
    // check the automatically pushed status
    if (!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pPushedStatusPipe))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
       
    // and the pullable status
    if (!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pPrinterStatusFile))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find first expected printer state and temperature again" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
     
    ps._state  = HomingState;
    ps._temperature = 42;
    
    // check status again (should not have changed)
    if (!ExpectedStatus(STATE_NAME(PrintingLayerState), "3.14159", _pPrinterStatusFile))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find unchanged printer state and temperature" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // send an update event with the new status
    ((ICallback*)&net)->Callback(PrinterStatusUpdate, EventData(ps));
    
    // check the automatically pushed status
    if (!ExpectedStatus(STATE_NAME(HomingState), "42", _pPushedStatusPipe))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // and the pullable status
    if (!ExpectedStatus(STATE_NAME(HomingState), "42", _pPrinterStatusFile))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (NetworkIFUT) message=failed to find new printer state and temperature again" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
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
        
    return (mainReturnValue);
}
