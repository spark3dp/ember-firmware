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
   for (
    PrinterStateMachine::state_iterator pLeafState = psm.state_begin();
    pLeafState != psm.state_end(); ++pLeafState )
  {
     if(strstr(typeid(*pLeafState).name(), expected) != NULL)
        return true;
     else
     {
       std::cout << "expected " << expected << " but actual state was " 
                                << typeid( *pLeafState ).name() << std::endl;
       std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=unexpected state" << std::endl;
       return false;
     }
  }
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
  
  printf("\n");
}

void test1() {
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    printf("\tabout to instantiate printer\n");
    PrinterStateMachine psm;
    printf("\tabout to initiate printer\n");
    psm.initiate();
    
    if(!ConfimExpectedState(psm, "Initializing"))
        return;
    
    printf("\tabout to process sleep event\n");
    psm.process_event(EvSleep());

    if(!ConfimExpectedState(psm, "Sleeping"))
        return;

    printf("\tabout to process wake event\n");    
    psm.process_event(EvWake());

    if(!ConfimExpectedState(psm, "Initializing"))
        return;

    printf("\tabout to process reset event\n");
    psm.process_event(EvReset());

    if(!ConfimExpectedState(psm, "Initializing"))
        return;    
    
    printf("\tabout to process door opened event\n");
    psm.process_event(EvDoorOpened());
    
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;

    printf("\tabout to process door closed event\n");    
    psm.process_event(EvDoorClosed());
    
    if(!ConfimExpectedState(psm, "Initializing"))
        return;     
    
    printf("\tabout to process door opened event again\n");
    psm.process_event(EvDoorOpened());
    
    if(!ConfimExpectedState(psm, "DoorOpen"))
        return;   

    printf("\tabout to process reset event again\n");
    psm.process_event(EvReset());
    
    if(!ConfimExpectedState(psm, "Initializing"))
        return; 
    
    printf("\tabout to shut down\n");
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

