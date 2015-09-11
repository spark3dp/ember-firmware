//  File:   EventHandlerUT.cpp
//  Tests EventHandler
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

#include <iostream>

#include "ICallback.h"
#include "EventType.h"
#include "PrinterStatusQueue.h"
#include "PrinterStatus.h"
#include "EventHandler.h"

// Simple C++ Test Suite
int mainReturnValue = EXIT_SUCCESS;

// Proxy for a second UI class, for test purposes
class UIProxy : public ICallback
{
public:    
    int _numCallbacks;
    
    UIProxy() : _numCallbacks(0) {}
    
private:    
    void Callback(EventType eventType, const EventData& data)
    {    
        PrinterStatus ps;
        switch(eventType)
        {                
            case PrinterStatusUpdate:
                _numCallbacks++;
                ps = data.Get<PrinterStatus>();
                std::cout << "UI2: got print status: layer " << 
                        ps._currentLayer <<
                        ", seconds left: " << 
                        ps._estimatedSecondsRemaining 
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
    statusQueue.Push(PrinterStatus());

    // run event loop for finite number of iterations
    // only possible in debug configuration
    eh.Begin(100);
    
    // when run against DEBUG build, check that we got the expected number of 
    // callbacks
    if (ui1._numCallbacks == 1 && 
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

