/* 
 * File:   EventHandlerUT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 1, 2014, 4:23:21 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/timerfd.h>

#include <Event.h>
#include "EventHandler.h"
#include "PrintEngine.h"

/*
 * Simple C++ Test Suite
 */

/// proxy for the PrintEngine, for test purposes
class PEProxy : public CallbackInterface
{
private:    
    int layer;
    int remaining;
    int _timer1FD;
    
public:
    PEProxy() :
     _gotInterrupt(false),
    layer(0),
    remaining(1000)
    {
        //TODO: set up a 1s periodic timer and set its fd into the EventHandler
        // create timers that use file descriptors
        _timer1FD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
        if (_timer1FD < 0)
        {
            printf("\nunable to create timer 1\n");
            exit(1);
        }

        struct itimerspec timer1Value;
        timer1Value.it_value.tv_sec = 2;
        timer1Value.it_value.tv_nsec = 0;
        timer1Value.it_interval.tv_sec = 2; // automatically repeat
       // timer1Value.it_interval.tv_sec = 0; // don't automatically repeat
        timer1Value.it_interval.tv_nsec =0;

        // set relative timer
        if (timerfd_settime(_timer1FD, 0, &timer1Value, NULL) == -1)
        {
            printf("couldn't set timer 1\n");
        }
    }
     
    bool _gotInterrupt;
    
    void callback(EventType eventType, void* data)
    {
        switch(eventType)
        {
            case ButtonInterrupt:
               _buttonCallback(data);
               break;
               
            case MotorInterrupt:
                _motorCallback(data);
                break;
                
            case DoorInterrupt:
                _doorCallback(data);
                break;
                
            case PrintEnginePulse:
                std::cout << "PE got pulse" << std::endl;
                SendStatusUpdate();
                break;
                
            default:
                // handle impossible case
                break;
        }
    }
    
    int GetTimerFD()
    {
        return _timer1FD;
    }
    
private:    
    
    void _buttonCallback(void*)
    {
        std::cout << "PE got button callback" << std::endl;
        _gotInterrupt = true;
    }
    
    void _motorCallback(void*)
    {
        std::cout << "PE got motor callback" << std::endl;
        _gotInterrupt = true;        
    }
    
    void _doorCallback(void*)
    {
        std::cout << "PE got door callback" << std::endl;
        _gotInterrupt = true;        
    }


    void SendStatusUpdate()
    {
        struct PrinterStatus ps;
        ps._currentLayer = layer++;
        ps._estimatedSecondsRemaining = remaining--;
        
        // TODO: send status info out the PE status pipe
    }
    
    // TODO
    // arrange to UT hardware interrupts by hardwiring one to a spare output
    // driven by the test SW itself
    
    // needs to set up delay timer & motor timeout timer, set the FDs for each, 
    // subscribe to those timers, then set the timers and see if the events come in time
    
    // needs to open FIFO for PrinterStatus, set its read fd into the EventHandler,
    // then write status to it (try 2 PriterStatus objects, written one right after the other,
    // and see if EH can handle those properly)
    
    // try driving status based on a separate timer (of which the EH is oblivious) as well as via individual calls
};

/// Proxy for a UI class, for test purposes
class UIProxy : public CallbackInterface
{
    // needs to subscribe to PrinterStatus events & make sure we always read the latest value
    
    
    void callback(EventType eventType, void* data)
    {     
        switch(eventType)
        {
            case ButtonInterrupt:
               _buttonCallback(data);
               break;
               
            case MotorInterrupt:
                _motorCallback(data);
                break;
                
            case DoorInterrupt:
                _doorCallback(data);
                break;
                
            case PrinterStatusUpdate:
                std::cout << "UI: got print status: layer" << 
                        ((PrinterStatus*)data)->_currentLayer <<
                        ", seconds left: " << 
                        ((PrinterStatus*)data)->_estimatedSecondsRemaining 
                        << std::endl;
                break;
                
            default:
                // handle impossible case
                break;
        }
    }
    void _buttonCallback(void*)
    {
        std::cout << "UI got button callback" << std::endl;
    }
    
    void _motorCallback(void*)
    {
        std::cout << "UI got motor callback" << std::endl;     
    }
    
    void _doorCallback(void*)
    {
        std::cout << "UI got door callback" << std::endl;    
    }

};

void test1() {
    std::cout << "EventHandlerUT test 1" << std::endl;
    
    EventHandler eh;
    PEProxy pe;
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetTimerFD()); // may make more sense here to pass eh in to PE, for it to set fd's as it sees fit
    eh.Subscribe(PrintEnginePulse, &pe);
    
    UIProxy ui;
    eh.Subscribe(MotorInterrupt, &ui);
    eh.Subscribe(ButtonInterrupt, &ui);
    eh.Subscribe(DoorInterrupt, &ui);
//    eh.Subscribe(PrinterStatusUpdate, &ui);

    eh.Begin();
    
}

void test2() {
    std::cout << "EventHandlerUT test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (EventHandlerUT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% EventHandlerUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (EventHandlerUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (EventHandlerUT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (EventHandlerUT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (EventHandlerUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

