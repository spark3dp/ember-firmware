/* 
 * File:   main.cpp
 * Author: Richard Greene
 *
 * Created on March 26, 2014, 4:01 PM
 */

#include <iostream>

#include <PrintEngine.h>
#include <EventHandler.h>

using namespace std;

/// Proxy for a UI class, for test purposes
class UIProxy : public ICallback
{ 
public:    
    int _numCallbacks;
    
    UIProxy() : _numCallbacks(0) {}
    
private:
    void Callback(EventType eventType, void* data)
    {     
        PrinterStatus* pPS;
        switch(eventType)
        {               
            case PrinterStatusUpdate:
                _numCallbacks++;
                pPS = (PrinterStatus*)data;
                std::cout << "UI proxy got printer status: " <<
                        pPS->_state << 
                        ", layer " << 
                        pPS->_currentLayer <<
                        ", seconds left: " << 
                        pPS->_estimatedSecondsRemaining 
                        << std::endl;
                break;
                
            default:
                HandleImpossibleCase(eventType);
                break;
        }
    }
};
 

int main(int argc, char** argv) 
{
    // create an event handler
    EventHandler eh;
    
    // connect it to a print engine that uses real hardware
    PrintEngine pe(true);
        
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
    
    eh.Subscribe(Keyboard, &pe);    
    
    // also connect a UI proxy
    UIProxy ui;
    // subscribe to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);
    
    // start the print engine's state machine
    pe.Begin();
    // begin handling events
    eh.Begin();
    
    return 0;
}

