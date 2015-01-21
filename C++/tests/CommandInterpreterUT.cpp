/* 
 * File:   CommandInterpreterUT.cpp
 * Author: Richard Greene
 *
 * Created on Jun 12, 2014, 11:51:33 AM
 */

#include <stdlib.h>
#include <iostream>

#include <CommandInterpreter.h>
#include <Hardware.h>


/*
 * Simple C++ Test Suite
 */
int mainReturnValue = EXIT_SUCCESS;
Command expected;
bool handled = false;

const char* expectedErrorMsg;
bool gotExpectedError = false;

class TestTarget: public ICommandTarget
{
    void Handle(Command command)
    {
        if(command != expected)
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
        if(strcmp(expectedErrorMsg, baseMsg) != 0)
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
    if(!handled)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=command not handled: " << 
                      expected <<  std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    handled = false;
}

void CheckNotHandled()
{
    if(handled)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (CommandInterpreterUT) message=illegal command handled" << 
                      std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    handled = false;
    if(!gotExpectedError)
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
    char textCmd[32];
    strcpy(textCmd, "Start");
    expected = Start;
    cmdInterp.Callback(UICommand, textCmd);
    CheckHandled(expected);
        
    strcpy(textCmd, "reSUme");
    expected = Resume;
    cmdInterp.Callback(Keyboard, textCmd);
    CheckHandled(expected);
    
    strcpy(textCmd, "PAUSE");
    expected = Pause;
    cmdInterp.Callback(Keyboard, textCmd);
    CheckHandled(expected);
    
    // check that illegal commands are not handled   
    strcpy(textCmd, "garbageIn");
    expectedErrorMsg = ERR_MSG(UnknownTextCommand);
    cmdInterp.Callback(UICommand, textCmd);
    CheckNotHandled();
    
    strcpy(textCmd, "Paws");
    expectedErrorMsg = ERR_MSG(UnknownTextCommand);
    cmdInterp.Callback(Keyboard, textCmd);
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

