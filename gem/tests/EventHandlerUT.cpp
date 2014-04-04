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
#include <sys/stat.h>
#include <fcntl.h>

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
    int _statusReadFD, _statusWriteFd;
    
public:
    PEProxy() :
     _gotInterrupt(false),
    layer(0),
    remaining(1000),
    _statusReadFD(-1),
    _statusWriteFd(-1)
    {
        // PE "owns" the pulse timer so it can enable and diasble it as needed
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
        
        // PE also owns the status update FIFO
        char pipeName[] = "/tmp/PrinterStatusPipe";
        // delete the file if it exists already
        char command[100];
        sprintf(command, "rm %s", pipeName);
        system(command);
        if (mkfifo(pipeName, 0666) < 0) {
          perror("Error creating the named pipe");
          return;
        }
        // Open both ends within this process in on-blocking mode
        // Must do like this otherwise open call will wait
        // till other end of pipe is opened by another process
        _statusReadFD = open(pipeName, O_RDONLY|O_NONBLOCK);
        _statusWriteFd = open(pipeName, O_WRONLY|O_NONBLOCK);
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
    
    int GetStatusUpdateFD()
    {
        return _statusReadFD;
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
        if(_statusWriteFd >= 0)
        {
            struct PrinterStatus ps;
            ps._currentLayer = layer++;
            ps._estimatedSecondsRemaining = remaining--;

            // send status info out the PE status pipe
            lseek(_statusWriteFd, 0, SEEK_SET);
            write(_statusWriteFd, &ps, sizeof(struct PrinterStatus));
        }
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

/// Proxy for a second UI class, for test purposes
class UI2Proxy : public CallbackInterface
{
    // needs to subscribe to PrinterStatus events & make sure we always read the latest value
    
    
    void callback(EventType eventType, void* data)
    {     
        switch(eventType)
        {                
            case PrinterStatusUpdate:
                std::cout << "UI2: got print status: layer" << 
                        ((PrinterStatus*)data)->_currentLayer <<
                        ", seconds left: " << 
                        ((PrinterStatus*)data)->_estimatedSecondsRemaining 
                        << std::endl;
                break;
        }
    }   
};


void test1() {
    std::cout << "EventHandlerUT test 1" << std::endl;
    
    EventHandler eh;
    PEProxy pe;
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    UIProxy ui;
    eh.Subscribe(MotorInterrupt, &ui);
    eh.Subscribe(ButtonInterrupt, &ui);
    eh.Subscribe(DoorInterrupt, &ui);
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);

    UI2Proxy ui2;
    eh.Subscribe(PrinterStatusUpdate, &ui2);
    
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

