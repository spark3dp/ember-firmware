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
#include <PrinterStateMachine.h>

/*
 * Simple C++ Test Suite
 * 
 * 
 */

int mainReturnValue = EXIT_SUCCESS;

/// method to determine if we're in the expected state
/// Note: it doesn't work for orthogonal states
bool ConfimExpectedState( const PrinterStateMachine* pPSM , const char* expected)
{   
    const char* name;
    
    for (PrinterStateMachine::state_iterator pLeafState = pPSM->state_begin();
         pLeafState != pPSM->state_end(); ++pLeafState )
    {
        name = typeid(*pLeafState).name();
        if(strstr(name, expected) != NULL)
            return true;
    }
    // here we must not have found a match, in any orthogonal region
    std::cout << "expected " << expected << " but actual state was " 
                             << name << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=unexpected state" << std::endl;
    mainReturnValue = EXIT_FAILURE;
    return false;
}

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
    eh.Subscribe(RotationInterrupt, &pe);
    
    // subscribe to timer events
    eh.SetFileDescriptor(ExposureEnd, pe.GetExposureTimerFD());
    eh.Subscribe(ExposureEnd, &pe);
    
    eh.SetFileDescriptor(MotorTimeout, pe.GetMotorTimeoutTimerFD());
    eh.Subscribe(MotorTimeout, &pe);
    
    // also connect a UI proxy
    UIProxy ui;
    // subscribe to printer status events
    eh.SetFileDescriptor(PrinterStatusUpdate, pe.GetStatusUpdateFD()); 
    eh.Subscribe(PrinterStatusUpdate, &ui);
    
    pe.Begin();
    if(!ConfimExpectedState(pe.GetStateMachine(), "Homing"))
        return;
    
    // Here we'd need to generate events, to make sure they were handled 
    // correctly.  But since we need hardware to generate interrupts to signal 
    // motor command completion, it's not clear how this could work in a UT.
    // Also, once we call EventHandler.Begin(), we can't do anything else from 
    // this thread.
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_EH_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PE_EH_IT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PE_EH_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

