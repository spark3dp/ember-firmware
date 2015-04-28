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
#include <EventHandler.h>
#include <PrinterStatus.h>
#include <Filenames.h>
#include <MotorController.h>

/*
 * Simple C++ Test Suite
 */
int mainReturnValue = EXIT_SUCCESS;

/// proxy for the PrintEngine, for test purposes
class PEProxy : public ICallback
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
        // PE also "owns" the status update FIFO
        // don't recreate the FIFO if it exists already
        if (access(PRINTER_STATUS_PIPE, F_OK) == -1) {
            if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0) {
              perror("Error creating the named pipe");
              return;
            }
        }
        // Open both ends within this process in on-blocking mode
        // Must do like this otherwise open call will wait
        // till other end of pipe is opened by another process
        _statusReadFD = open(PRINTER_STATUS_PIPE, O_RDONLY|O_NONBLOCK);
        _statusWriteFd = open(PRINTER_STATUS_PIPE, O_WRONLY|O_NONBLOCK);
    }
     
    bool _gotInterrupt;
    
    void Callback(EventType eventType, void* data)
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
                
            case RotationInterrupt:
                _jamCallback(data);
                break;
                
            default:
                HandleImpossibleCase(eventType);
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
        
    void _buttonCallback(void*)
    {
        std::cout << "PE: got button callback" << std::endl;
        _gotInterrupt = true;
    }
    
    void _motorCallback(void*)
    {
        std::cout << "PE: got motor callback" << std::endl;
        _gotInterrupt = true;        
    }
    
    void _doorCallback(void*)
    {
        std::cout << "PE: got door callback" << std::endl;
        _gotInterrupt = true;        
    }

    void _jamCallback(void*)
    {
        std::cout << "PE: got rotation jam callback" << std::endl;
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
            write(_statusWriteFd, &ps, sizeof(struct PrinterStatus));
        }
    }
};

/// Proxy for a UI class, for test purposes
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
            case ButtonInterrupt:
                _numCallbacks++;
               _buttonCallback(data);
               break;
               
            case MotorInterrupt:
                _numCallbacks++;
                _motorCallback(data);
                break;
                
            case DoorInterrupt:
                _numCallbacks++;
                _doorCallback(data);
                break;
                
            case RotationInterrupt:
                _numCallbacks++;
                _jamCallback(data);
                break;
                
            case PrinterStatusUpdate:
                _numCallbacks++;
                std::cout << "UI: got print status: layer " << 
                        ((PrinterStatus*)data)->_currentLayer <<
                        ", seconds left: " << 
                        ((PrinterStatus*)data)->_estimatedSecondsRemaining 
                        << std::endl;
                break;
                
            default:
                HandleImpossibleCase(eventType);
                break;
        }
    }
    void _buttonCallback(void*)
    {
        std::cout << "UI: got button callback" << std::endl;
    }
    
    void _motorCallback(void*)
    {
        std::cout << "UI: got motor callback" << std::endl;     
    }
    
    void _doorCallback(void*)
    {
        std::cout << "UI: got door callback" << std::endl;    
    }

    void _jamCallback(void*)
    {
        std::cout << "UI: got rotation jam callback" << std::endl;    
    }

};

/// Proxy for a second UI class, for test purposes
class UI2Proxy : public ICallback
{
public:    
    int _numCallbacks;
    
    UI2Proxy() : _numCallbacks(0) {}
    
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
                HandleImpossibleCase(eventType);
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
    eh.Subscribe(RotationInterrupt, &pe);
    
    UIProxy ui;
    UI2Proxy ui2;
    
    eh.Subscribe(MotorInterrupt, &ui);
    eh.Subscribe(ButtonInterrupt, &ui);
    eh.Subscribe(DoorInterrupt, &ui);
    eh.Subscribe(RotationInterrupt, &ui);
    
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);

    eh.Subscribe(PrinterStatusUpdate, &ui2);

    int numIterations = 100;
    
    // generate some events to exercise their callbacks
    pe.SendStatusUpdate();
    unsigned char status = MC_SUCCESS;
    ((ICallback*)&ui)->Callback(MotorInterrupt, &status);

#ifdef DEBUG    
    eh.Begin(numIterations);
#else
    eh.Begin();
#endif
    
    // when run against DEBUG build, check that we got the expected number of 
    // callbacks
    if(ui._numCallbacks == 2 && 
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

