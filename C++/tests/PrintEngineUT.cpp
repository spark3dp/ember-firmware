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
#include <Settings.h>
#include <stdio.h>
#include <string.h>

std::string tempDir;

void Setup()
{
    // Create a temp directory for print data (slice images)
    tempDir = CreateTempDir();
    
    // Copy slice images into the temp directory
    Copy("/smith/test_resources/slices/slice_1.png", tempDir + "/slice_1.png");
    Copy("/smith/test_resources/slices/slice_2.png", tempDir + "/slice_2.png");
    
    // Configure the temp directory as the print data directory
    SETTINGS.Set(PRINT_DATA_DIR, tempDir);
}

void TearDown()
{
    SETTINGS.Restore(PRINT_DATA_DIR);
    RemoveDir(tempDir);
}

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
    unsigned char status = 0;
    char doorState = '1'; // door closed
    
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    std::cout << "\tabout to instantiate & initiate printer" << std::endl;
    // set up print engine for a single layer, 
    // that will also start up its state machine,
    // but don't require use of real hardware
    PrintEngine pe(false);
    pe.SetNumLayers(2);
    pe.Begin();
        
    PrinterStateMachine* pPSM = pe.GetStateMachine();
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return;

    std::cout << "\tabout to process reset event" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return;    
    
    std::cout << "\tabout to process door opened event" << std::endl;
    doorState = '0';
    ((ICallback*)&pe)->Callback(DoorInterrupt, &doorState); 
    if(!ConfimExpectedState(pPSM, DOOR_OPEN_STATE))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    doorState = '1';
    ((ICallback*)&pe)->Callback(DoorInterrupt, &doorState); 
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return;     
    
    std::cout << "\tabout to process door opened event again" << std::endl;
    pPSM->process_event(EvDoorOpened());
    if(!ConfimExpectedState(pPSM, DOOR_OPEN_STATE))
        return;   

    std::cout << "\tabout to process reset event again" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return; 
    
    std::cout << "\tabout to test main path" << std::endl; 
    pPSM->process_event(EvInitialized());
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return;    
    
    status = 0;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, HOME_STATE))
        return;   

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, DOOR_OPEN_STATE))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, HOME_STATE))
        return;  
    
    ((ICommandTarget*)&pe)->Handle(Start);
    if(!ConfimExpectedState(pPSM, PRINT_SETUP_STATE))
        return; 
    
    // got first setting
    pPSM->process_event(EvGotSetting());
    if(!ConfimExpectedState(pPSM, PRINT_SETUP_STATE))
        return;
    
    // got second setting
    pPSM->process_event(EvGotSetting());
    if(!ConfimExpectedState(pPSM, MOVING_TO_START_STATE))
        return; 
    
    std::cout << "\tabout to start printing" << std::endl;
    pPSM->process_event(EvAtStartPosition());
    if(!ConfimExpectedState(pPSM, EXPOSING_STATE))
        return;
    
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return; 

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, DOOR_OPEN_STATE))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return;  
    
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, EXPOSING_STATE))
        return; 
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return; 
    
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, EXPOSING_STATE))
        return; 
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return; 
        
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, ENDING_PRINT_STATE))
        return; 
    
    pPSM->process_event(EvPrintEnded());
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 

    std::cout << "\tabout to cancel" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Cancel);
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 

    std::cout << "\tabout to process an error" << std::endl;
    pPSM->process_event(EvError());
    if(!ConfimExpectedState(pPSM, IDLE_STATE))
        return; 

    //get back to where we can test pause/resume
    std::cout << "\tabout to start printing again" << std::endl;
    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 
    
    pPSM->process_event(EvAtHome());
    // goes straight to print setup, without second start command
    if(!ConfimExpectedState(pPSM, PRINT_SETUP_STATE))
        return;  
    
    // got first setting, via the ICallback interface
    status = 0;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, PRINT_SETUP_STATE))
        return;
       
    // got second setting, via the ICallback interface
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, MOVING_TO_START_STATE))
        return; 

    pPSM->process_event(EvAtStartPosition());
    if(!ConfimExpectedState(pPSM, EXPOSING_STATE))
        return; 
    
    ((ICallback*)&pe)->Callback(ExposureEnd, NULL);
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return; 

    // test pause/resume
    std::cout << "\tabout to pause" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Pause);
    if(!ConfimExpectedState(pPSM, PAUSED_STATE))
        return; 
        
    std::cout << "\tabout to resume" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Resume);
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return;  

    std::cout << "\tabout to pause and resume using button1 generic command" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(StartPauseOrResume);
    if(!ConfimExpectedState(pPSM, PAUSED_STATE))
        return;

    ((ICommandTarget*)&pe)->Handle(StartPauseOrResume);
    if(!ConfimExpectedState(pPSM, SEPARATING_STATE))
        return;
    
    std::cout << "\tabout to handle last layer" << std::endl;
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, ENDING_PRINT_STATE))
        return;  

    pPSM->process_event(EvPrintEnded());
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 
    
    std::cout << "\thandle a non-fatal error" << std::endl;
    status = 0x55;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 
    
    std::cout << "\thandle another non-fatal error" << std::endl;
    status = 0x55;
    ((ICallback*)&pe)->Callback(PrinterStatusUpdate, NULL);
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return;     

    std::cout << "\thandle a fatal error" << std::endl;
    status = 0xFF;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, IDLE_STATE))
        return; 

    pPSM->process_event(EvStartPrint());   
    if(!ConfimExpectedState(pPSM, HOMING_STATE))
        return; 
    
    std::cout << "\thandle another fatal error" << std::endl;
    ((ICallback*)&pe)->Callback(MotorTimeout, NULL);
    if(!ConfimExpectedState(pPSM, IDLE_STATE))
        return; 
    
    std::cout << "\ttest reset" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Reset);
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return; 
    
    std::cout << "\ttest refreshing settings" << std::endl;
    ((ICommandTarget*)&pe)->Handle(RefreshSettings);
    if(!ConfimExpectedState(pPSM, INITIALIZING_STATE))
        return; 
    
    std::cout << "\tabout to shut down" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintEngineUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PrintEngineUT)" << std::endl;
    Setup();
    test1();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 test1 (PrintEngineUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

