/* 
 * File:   PrintEngineUT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 8, 2014, 4:13:10 PM
 */

#include <stdlib.h>
#include <iostream>
#include <PrintEngine.h>
#include <stdio.h>
#include <string.h>

/*
 * Simple C++ Test Suite
 */


/// method to determine if we're in the expected state
/// Note: it doesn't work for orthogonal states
bool ConfimExpectedState( const PrinterStateMachine& psm , const char* expected)
{   
    const char* name;
    
    for (PrinterStateMachine::state_iterator pLeafState = psm.state_begin();
         pLeafState != psm.state_end(); ++pLeafState )
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

void DisplayStateConfiguration( const PrinterStateMachine & psm )
{
  printf("\t\tstate config = ");
  char region = 'a';

  for (
    PrinterStateMachine::state_iterator pLeafState = psm.state_begin();
    pLeafState != psm.state_end(); ++pLeafState )
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
        
    std::cout << "\tabout to instantiate printer" << std::endl;
    PrinterStateMachine psm;
    std::cout << "\tabout to initiate printer" << std::endl;
    psm.initiate();
    if(!ConfimExpectedState(psm, "Initializing"))
        return;
    
    std::cout << "\tabout to process sleep event" << std::endl;
    psm.process_event(EvSleep());
    if(!ConfimExpectedState(psm, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    psm.process_event(EvWake());
    if(!ConfimExpectedState(psm, "Initializing"))
        return;

    std::cout << "\tabout to process reset event" << std::endl;
    psm.process_event(EvReset());
    if(!ConfimExpectedState(psm, "Initializing"))
        return;    
    
    std::cout << "\tabout to process door opened event" << std::endl;
    psm.process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    psm.process_event(EvDoorClosed());
    if(!ConfimExpectedState(psm, "Initializing"))
        return;     
    
    std::cout << "\tabout to process door opened event again" << std::endl;
    psm.process_event(EvDoorOpened());
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;   

    std::cout << "\tabout to process reset event again" << std::endl;
    psm.process_event(EvReset());
    if(!ConfimExpectedState(psm, "Initializing"))
        return; 
    
    std::cout << "\tabout to test main path" << std::endl; 
    psm.process_event(EvInitialized());
    if(!ConfimExpectedState(psm, "Homing"))
        return; 
    
    psm.process_event(EvAtHome());
    if(!ConfimExpectedState(psm, "Home"))
        return; 
    
    std::cout << "\tabout to process sleep event" << std::endl;
    psm.process_event(EvSleep());
    if(!ConfimExpectedState(psm, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    psm.process_event(EvWake());
    if(!ConfimExpectedState(psm, "Home"))
        return;    

    std::cout << "\tabout to process door opened event" << std::endl;
    psm.process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    psm.process_event(EvDoorClosed());    
    if(!ConfimExpectedState(psm, "Home"))
        return;  
    
    psm.process_event(EvStartPrint());
    if(!ConfimExpectedState(psm, "MovingToStartPosition"))
        return; 

    std::cout << "\tabout to start printing" << std::endl;
    psm.process_event(EvAtStartPosition());
    if(!ConfimExpectedState(psm, "Exposing"))
        return;
    
    psm.process_event(EvPulse());
    if(!ConfimExpectedState(psm, "Exposing"))
        return; 

    psm.process_event(EvExposed());
    if(!ConfimExpectedState(psm, "Separating"))
        return;
    
    psm.process_event(EvPulse());
    if(!ConfimExpectedState(psm, "Separating"))
        return; 

    std::cout << "\tabout to process sleep event" << std::endl;
    psm.process_event(EvSleep());
    if(!ConfimExpectedState(psm, "Sleeping"))
        return;

    std::cout << "\tabout to process wake event" << std::endl;    
    psm.process_event(EvWake());
    if(!ConfimExpectedState(psm, "Separating"))
        return;    

    std::cout << "\tabout to process door opened event" << std::endl;
    psm.process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    psm.process_event(EvDoorClosed());    
    if(!ConfimExpectedState(psm, "Separating"))
        return;  
    
    psm.process_event(EvSeparated());
    if(!ConfimExpectedState(psm, "MovingToLayer"))
        return; 
    
    psm.process_event(EvPulse());
    if(!ConfimExpectedState(psm, "MovingToLayer"))
        return; 

    psm.process_event(EvAtLayer());
    if(!ConfimExpectedState(psm, "Exposing"))
        return; 

    std::cout << "\tabout to cancel" << std::endl;
    psm.process_event(EvCancel());
    if(!ConfimExpectedState(psm, "Homing"))
        return; 

    std::cout << "\tabout to process an error" << std::endl;
    psm.process_event(EvError());
    if(!ConfimExpectedState(psm, "Idle"))
        return; 

    std::cout << "\tabout to start printing again" << std::endl;
    psm.process_event(EvStartPrint());
    if(!ConfimExpectedState(psm, "Homing"))
        return; 
    
    //get back to where we can test pause/resume
    psm.process_event(EvAtHome());
    if(!ConfimExpectedState(psm, "Home"))
        return; 

    psm.process_event(EvStartPrint());
    if(!ConfimExpectedState(psm, "MovingToStartPosition"))
        return; 

    psm.process_event(EvAtStartPosition());
    if(!ConfimExpectedState(psm, "Exposing"))
        return; 
    
    psm.process_event(EvExposed());
    if(!ConfimExpectedState(psm, "Separating"))
        return; 

    // test pause/resume
    std::cout << "\tabout to pause" << std::endl;
    psm.process_event(EvPause());
    if(!ConfimExpectedState(psm, "Paused"))
        return; 
        
    std::cout << "\tabout to resume" << std::endl;
    psm.process_event(EvResume());
    if(!ConfimExpectedState(psm, "Separating"))
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

