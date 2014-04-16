/* 
 * File:   PE_EH_IT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 14, 2014, 4:05:38 PM
 */

#include <stdlib.h>
#include <iostream>

#include <PrintEngine.h>
#include <EventHandler.h>

/*
 * Simple C++ Test Suite
 * 
 * 
 */

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
};
 

void test1() {
    std::cout << "PrintEngine/EventHandler integration test 1" << std::endl;
    
    // create an event handler
    EventHandler eh;
    
    // connect it to a print engine 
    PrintEngine pe(false);
    // subscribe to interrupt events
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    // subscribe to timer events
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetPulseTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    // also connect a UI proxy
    UIProxy ui;
    // subscribe to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);
    
    // start handling events
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_EH_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PE_EH_IT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PE_EH_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

