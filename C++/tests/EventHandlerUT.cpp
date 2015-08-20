/* 
 * File:   EventHandlerUT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 1, 2014, 4:23:21 PM
 */

#include <iostream>

#include "ICallback.h"
#include "EventType.h"
#include "PrinterStatusPipe.h"
#include "PrinterStatus.h"
#include "EventHandler.h"

/*
 * Simple C++ Test Suite
 */
int mainReturnValue = EXIT_SUCCESS;

/// Proxy for a second UI class, for test purposes
class UIProxy : public ICallback
{
public:    
    int _numCallbacks;
    
    UIProxy() : _numCallbacks(0) {}
    
private:    
    void Callback(EventType eventType, void* data)
    {     
        switch(eventType)
        {                
            case PrinterStatusUpdate:
                _numCallbacks++;
                std::cout << "UI2: got print status: layer " << 
                        ((PrinterStatus*)data)->_currentLayer <<
                        ", seconds left: " << 
                        ((PrinterStatus*)data)->_estimatedSecondsRemaining 
                        << std::endl;
                break;
                
            default:
                std::cout << "UI2: impossible case" << std::endl;
                break;
        }
    }   
};


void test1() {
    std::cout << "EventHandlerUT test 1" << std::endl;
    
    EventHandler eh;
    
    PrinterStatusQueue statusQueue;

    UIProxy ui1;
    UIProxy ui2;
   
    eh.AddEvent(PrinterStatusUpdate, &statusQueue);
    
    eh.Subscribe(PrinterStatusUpdate, &ui1);
    eh.Subscribe(PrinterStatusUpdate, &ui2);

    // generate an event
    PrinterStatus status;
    statusQueue.Push(&status);

    // run event loop for finite number of iterations
    // only possible in debug configuration
    eh.Begin(100);
    
    // when run against DEBUG build, check that we got the expected number of 
    // callbacks
    if(ui1._numCallbacks == 1 && 
       ui2._numCallbacks == 1)
    {
        // passed
        std::cout << "%TEST_PASSED% time=0 testname=test1 (EventHandlerUT) message=got expected number of callbacks" << std::endl;
    }
    else
    {
       // failed
       std::cout << "%TEST_FAILED% time=0 testname=test1 (EventHandlerUT) message=didn't get expected number of callbacks" << std::endl;
       mainReturnValue = EXIT_FAILURE;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% EventHandlerUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (EventHandlerUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (EventHandlerUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

