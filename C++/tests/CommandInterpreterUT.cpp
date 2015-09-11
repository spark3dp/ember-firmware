//  File:   CommandInterpreterUT.cpp
//  Tests CommandInterpreter
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

#include <CommandInterpreter.h>
#include <Hardware.h>


// Simple C++ Test Suite
int mainReturnValue = EXIT_SUCCESS;
Command expected;
bool handled = false;

const char* expectedErrorMsg;
bool gotExpectedError = false;

class TestTarget: public ICommandTarget
{
    void Handle(Command command)
    {
        if (command != expected)
        {
            std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=unexpected command.  " << 
                          "expected: " << expected << " but got: " << command << std::endl;
            mainReturnValue = EXIT_FAILURE;
        }
        else
        {
            std::cout << "got expected command: " << command << std::endl;
            handled = true;
        }
    }
    
    void HandleError(ErrorCode code, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX)
    {
        const char* baseMsg = ERR_MSG(code);
        // check for expected error
        if (strcmp(expectedErrorMsg, baseMsg) != 0)
        {
            std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=unexpected error.  " << 
                          "expected: " << expectedErrorMsg << " but got: " << baseMsg << std::endl;
            mainReturnValue = EXIT_FAILURE;
        }
        else
        {
            std::cout << "got expected error: " << expectedErrorMsg << std::endl;
            gotExpectedError = true;
        }
    }
};

void CheckHandled(Command expected)
{
    if (!handled)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=command not handled: " << 
                      expected <<  std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    handled = false;
}

void CheckNotHandled()
{
    if (handled)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=illegal command handled" << 
                      std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    handled = false;
    if (!gotExpectedError)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=expected error not found" << 
                      std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    gotExpectedError = false;
}

void test1() {
    std::cout << "CommandInterpreterUT test 1" << std::endl;
    TestTarget target;
    CommandInterpreter cmdInterp(&target);
    
    // check that text commands correctly handled
    expected = Start;
    cmdInterp.Callback(UICommand, EventData(std::string("Start")));
    CheckHandled(expected);
    
    expected = Resume;
    cmdInterp.Callback(Keyboard, EventData(std::string("reSUme")));
    CheckHandled(expected);
    
    expected = Pause;
    cmdInterp.Callback(Keyboard, EventData(std::string("PAUSE")));
    CheckHandled(expected);
    
    // check that illegal commands are not handled   
    expectedErrorMsg = ERR_MSG(UnknownTextCommand);
    cmdInterp.Callback(UICommand, EventData(std::string("garbageIn")));
    CheckNotHandled();
    
    expectedErrorMsg = ERR_MSG(UnknownTextCommand);
    cmdInterp.Callback(Keyboard, EventData(std::string("Paws")));
    CheckNotHandled();
}



int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% CommandInterpreterUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (CommandInterpreterUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (CommandInterpreterUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

