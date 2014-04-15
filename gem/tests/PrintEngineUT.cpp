/* 
 * File:   PrintEngineUT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 8, 2014, 4:13:10 PM
 */

#include <stdlib.h>
#include <iostream>
#include <PrintEngine.h>
#include <PrinterStateMachine.h>
#include <stdio.h>
#include <string.h>

/*
 * Simple C++ Test Suite
 */


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
    return false;
}

void DisplayStateConfiguration( const PrinterStateMachine* pPSM )
{
  printf("\t\tstate config = ");
  char region = 'a';

  for (
    PrinterStateMachine::state_iterator pLeafState = pPSM->state_begin();
    pLeafState != pPSM->state_end(); ++pLeafState )
  {
    std::cout << "Orthogonal region " << region << ": ";
   // std::cout << pLeafState->custom_dynamic_type_ptr< char >() << "\n";
    std::cout << typeid( *pLeafState ).name() << "\n";
    ++region;
  }
  
  std::cout << "" << std::endl;
}

void test1() {
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    // set up print engine for a single layer, 
    // that will also start up its state machine
    std::cout << "\tabout to instantiate & initiate printer" << std::endl;
    PrintEngine pe;
    pe.SetNumLayers(1);
        
    PrinterStateMachine* pPSM = pe.GetStateMachine();
    if(!ConfimExpectedState(pPSM, "Homing"))
        return;
    
    std::cout << "\tabout to process sleep event" << std::endl;
    pPSM->process_event(EvSleep());
    if(!ConfimExpectedState(pPSM, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    pPSM->process_event(EvWake());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return;

    std::cout << "\tabout to process reset event" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return;    
    
    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return;     
    
    std::cout << "\tabout to process door opened event again" << std::endl;
    pPSM->process_event(EvDoorOpened());
    if(!ConfimExpectedState(pPSM, "DoorOpen"))
        return;   

    std::cout << "\tabout to process reset event again" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return; 
    
    std::cout << "\tabout to test main path" << std::endl;    
    pPSM->process_event(EvAtHome());
    if(!ConfimExpectedState(pPSM, "Home"))
        return; 
    
    std::cout << "\tabout to process sleep event" << std::endl;
    pPSM->process_event(EvSleep());
    if(!ConfimExpectedState(pPSM, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    pPSM->process_event(EvWake());
    if(!ConfimExpectedState(pPSM, "Home"))
        return;    

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, "Home"))
        return;  
    
    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, "MovingToStartPosition"))
        return; 

    std::cout << "\tabout to start printing" << std::endl;
    pPSM->process_event(EvAtStartPosition());
    if(!ConfimExpectedState(pPSM, "Exposing"))
        return;
    
    pPSM->process_event(EvPulse());
    if(!ConfimExpectedState(pPSM, "Exposing"))
        return; 

    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, "Separating"))
        return;
    
    pPSM->process_event(EvPulse());
    if(!ConfimExpectedState(pPSM, "Separating"))
        return; 

    std::cout << "\tabout to process sleep event" << std::endl;
    pPSM->process_event(EvSleep());
    if(!ConfimExpectedState(pPSM, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    pPSM->process_event(EvWake());
    if(!ConfimExpectedState(pPSM, "Separating"))
        return;    

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, "Separating"))
        return;  
    
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, "MovingToLayer"))
        return; 
    
    pPSM->process_event(EvPulse());
    if(!ConfimExpectedState(pPSM, "MovingToLayer"))
        return; 

    pPSM->process_event(EvAtLayer());
    if(!ConfimExpectedState(pPSM, "Exposing"))
        return; 

    std::cout << "\tabout to cancel" << std::endl;
    pPSM->process_event(EvCancel());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return; 

    std::cout << "\tabout to process an error" << std::endl;
    pPSM->process_event(EvError());
    if(!ConfimExpectedState(pPSM, "Idle"))
        return; 

    std::cout << "\tabout to start printing again" << std::endl;
    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return; 
    
    //get back to where we can test pause/resume
    pPSM->process_event(EvAtHome());
    if(!ConfimExpectedState(pPSM, "Home"))
        return; 

    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, "MovingToStartPosition"))
        return; 

    pPSM->process_event(EvAtStartPosition());
    if(!ConfimExpectedState(pPSM, "Exposing"))
        return; 
    
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, "Separating"))
        return; 

    // test pause/resume
    std::cout << "\tabout to pause" << std::endl;
    pPSM->process_event(EvPause());
    if(!ConfimExpectedState(pPSM, "Paused"))
        return; 
        
    std::cout << "\tabout to resume" << std::endl;
    pPSM->process_event(EvResume());
    if(!ConfimExpectedState(pPSM, "Separating"))
        return;  

    std::cout << "\tabout to handle last layer" << std::endl;
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, "Homing"))
        return;  

    std::cout << "\tabout to shut down" << std::endl;
}

void test2() {
//    std::cout << "PrintEngineUT test 2" << std::endl;
//    std::cout << "%TEST_FAILED% time=0 testname=test2 (PrintEngineUT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintEngineUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PrintEngineUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PrintEngineUT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (PrintEngineUT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (PrintEngineUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

