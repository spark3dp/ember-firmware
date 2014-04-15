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

};
 

void test1() {
    std::cout << "PrintEngine/EventHandler integration test 1" << std::endl;
    
    // create an event handler
    EventHandler eh;
    
    // connect it to the print engine singleton
    PrintEngine pe;
    eh.Subscribe(MotorInterrupt, &pe);
    eh.Subscribe(ButtonInterrupt, &pe);
    eh.Subscribe(DoorInterrupt, &pe);
    
    eh.SetFileDescriptor(PrintEnginePulse, pe.GetPulseTimerFD()); 
    eh.Subscribe(PrintEnginePulse, &pe);
    
    // also connect a UI proxy
    UIProxy ui;
    eh.Subscribe(MotorInterrupt, &ui);
    eh.Subscribe(ButtonInterrupt, &ui);
    eh.Subscribe(DoorInterrupt, &ui);
    
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);
    
    
 
    
    // start handling events
}

void test2() {
//    std::cout << "PE_EH_IT test 2" << std::endl;
//    std::cout << "%TEST_FAILED% time=0 testname=test2 (PE_EH_IT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_EH_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PE_EH_IT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PE_EH_IT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (PE_EH_IT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (PE_EH_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

