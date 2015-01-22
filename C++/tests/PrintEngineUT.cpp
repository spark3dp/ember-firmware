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
#include <Hardware.h>
#include <Shared.h>

int mainReturnValue = EXIT_SUCCESS;

std::string tempDir;

#define NUM_ADDITIONAL_SETTINGS (26)

int g_initalHardwareRev;
int g_initalMotorFWRev;
int g_initialLayerThickness;

void Setup()
{
    // Create a temp directory for print data (slice images)
    tempDir = CreateTempDir();
    
    // Copy slice images into the temp directory
    Copy("resources/slices/slice_1.png", tempDir + "/slice_1.png");
    Copy("resources/slices/slice_2.png", tempDir + "/slice_2.png");
    
    // Configure the temp directory as the print data directory
    SETTINGS.Set(PRINT_DATA_DIR, tempDir);
    
    // record the HW rev setting, to be able to restore it at the end
    g_initalHardwareRev = SETTINGS.GetInt(HARDWARE_REV);
    // set the HW rev to test jamming detection 
    // (though we don't Save it here, all settings will get saved when we do 
    // other operations below)
    SETTINGS.Set(HARDWARE_REV, 1);
    
    // record the layer thickness setting
    g_initialLayerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
    // set it
}

void TearDown()
{
    // restore the settings to what they were before
    SETTINGS.Set(HARDWARE_REV, g_initalHardwareRev);
    SETTINGS.Set(LAYER_THICKNESS, g_initialLayerThickness);
    
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
    mainReturnValue = EXIT_FAILURE;
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

/// Common things we need to do after starting a print
void GoToStartPosition(PrintEngine* ppe)
{
    PrinterStateMachine* pPSM = ppe->GetStateMachine();
    
    // got first setting, via the ICallback interface
    unsigned char status = SUCCESS;
    ((ICallback*)ppe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(PrintSetupState)))
        return;
    
    // handle additional settings 
    for(int i = 0; i < NUM_ADDITIONAL_SETTINGS; i++)
    {
        pPSM->process_event(EvGotSetting());
        if(!ConfimExpectedState(pPSM, STATE_NAME(PrintSetupState)))
        return;            
    }
       
    // got last setting, via the ICallback interface
    ((ICallback*)ppe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToStartPositionState)))
        return; 
}

void test1() {  
    unsigned char status = 0;
      
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    std::cout << "\tabout to instantiate & initiate printer" << std::endl;
    
    // set up print engine for a single layer, 
    // that will also start up its state machine,
    // but don't require use of real hardware
    PrintEngine pe(false);
    pe.SetNumLayers(2);
    pe.Begin();
        
    PrinterStateMachine* pPSM = pe.GetStateMachine();
    if(!ConfimExpectedState(pPSM, STATE_NAME(InitializingState)))
        return;

    std::cout << "\tabout to process reset event" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, STATE_NAME(InitializingState)))
        return;    
    
    std::cout << "\tabout to process door opened event" << std::endl;
    unsigned char doorState = '1';
    ((ICallback*)&pe)->Callback(DoorInterrupt, &doorState); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    doorState = '0';
    ((ICallback*)&pe)->Callback(DoorInterrupt, &doorState); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(InitializingState)))
        return;     
    
    std::cout << "\tabout to process door opened event again" << std::endl;
    pPSM->process_event(EvDoorOpened());
    if(!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;   

    std::cout << "\tabout to process reset event again" << std::endl;
    pPSM->process_event(EvReset());
    if(!ConfimExpectedState(pPSM, STATE_NAME(InitializingState)))
        return; 
    
    pPSM->process_event(EvInitialized());
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;    
    
    status = 0;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;   
    
    std::cout << "\tabout to process show version event" << std::endl;
    pPSM->process_event(EvShowVersion()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(ShowingVersionState)))
        return;

    std::cout << "\tabout to process hide version event" << std::endl;
    pPSM->process_event(EvRightButton()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;  
    
    std::cout << "\tabout to test calibration procedure" << std::endl; 
    status = BTN2_HOLD;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(CalibrateState)))
        return;
    
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToCalibrationState)))
        return;
    
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(CalibratingState)))
        return; 
    
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(EndingCalibrationState)))
        return;
   
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    std::cout << "\tabout to test registration" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(StartRegistering);
    if(!ConfimExpectedState(pPSM, STATE_NAME(RegisteringState)))
        return; 
    
    ((ICommandTarget*)&pe)->Handle(RegistrationSucceeded);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    std::cout << "\tabout to test main path" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Start);
    if(!ConfimExpectedState(pPSM, STATE_NAME(PrintSetupState)))
        return; 
    
    GoToStartPosition(&pe);
        
    std::cout << "\tabout to start printing" << std::endl;
    // send EvAtStartPosition, via the ICallback interface
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
    
    status = '0';
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return; 
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    std::cout << "\tabout to handle resin tray jamming" << std::endl;
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(JammedState)))
        return; 
        
    // resume after jamming
    ((ICommandTarget*)&pe)->Handle(Resume);  
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    // send separated event again, but this time provide rotation interrupt
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;     
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
    
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 

    std::cout << "\tabout to process an error" << std::endl;
    pPSM->process_event(EvError());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 

    //get back to where we can test pause/resume
    std::cout << "\tabout to start printing again" << std::endl;
    pPSM->process_event(EvRightButton());
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvAtHome());
    // now needs second start command
    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, STATE_NAME(PrintSetupState)))
        return;  
    
    GoToStartPosition(&pe);
    
    // send EvAtStartPosition, via the ICallback interface
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    ((ICallback*)&pe)->Callback(ExposureEnd, NULL);
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    // test pause/resume
    std::cout << "\tabout to pause" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Pause);
    // requesting a pause while separating just sets a flag
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
    
    // send EvSeparated, via the ICallback interface, with rotation interrupt,
    // which should now start the pause & inspect process
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(RotatingForPauseState)))
        return; 

    pPSM->process_event(EvRotatedForPause());
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToPauseState)))
        return; 
    
    // complete pausing without setting the flag indicating we've moved up to 
    // the inspection position
    pPSM->process_event(EvAtPause());
    if(!ConfimExpectedState(pPSM, STATE_NAME(PausedState)))
        return; 

    std::cout << "\tabout to resume" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Resume);
    if(!ConfimExpectedState(pPSM, STATE_NAME(RotatingForResumeState)))
        return;      
    
    pPSM->process_event(EvRotatedForResume());  
    // since we hadn't lifted to the inspection position, we only needed that 
    // rotation to get back to the Exposing position
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;  

    std::cout << "\tabout to pause using right button" << std::endl; 
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    // requesting a pause while separating also just sets a flag
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
    
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(RotatingForPauseState)))
        return; 
    
    pPSM->process_event(EvRotatedForPause());
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToPauseState)))
        return; 
    
    // send EvAtPause, via the ICallback interface, in order to set flag
    // indicating we moved up to the inspection position
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(PausedState)))
        return; 

    
    std::cout << "\tabout to request cancel" << std::endl;
    status = BTN1_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 
    
    std::cout << "\tbut not confirm cancel" << std::endl;
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(RotatingForResumeState)))
        return;
    
    pPSM->process_event(EvRotatedForResume());
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToResumeState)))
        return; 

    pPSM->process_event(EvAtResume());    
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;  
    
    std::cout << "\tabout to request cancel again" << std::endl;
    pPSM->process_event(EvLeftButton());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 
    
    std::cout << "\tand try to confirm while motor is still moving" << std::endl;
    pPSM->process_event(EvLeftButton());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 

    std::cout << "\tdo confirm after motor has stopped" << std::endl;
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    pPSM->process_event(EvLeftButton());
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\thandle a non-fatal error" << std::endl;
    status = 0x55;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\thandle another non-fatal error" << std::endl;
    status = 0x55;
    ((ICallback*)&pe)->Callback(PrinterStatusUpdate, NULL);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;     

    std::cout << "\thandle a fatal error" << std::endl;
    status = 0xFF;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 
    
    std::cout << "\tabout to process show version event via left button hold" << std::endl;
    status = BTN1_HOLD;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ShowingVersionState)))
        return;

    std::cout << "\tabout to process hide version event again" << std::endl;
    pPSM->process_event(EvRightButton()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return;

    pPSM->process_event(EvRightButton());   
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\thandle another fatal error" << std::endl;
    ((ICallback*)&pe)->Callback(MotorTimeout, NULL);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 
    
    std::cout << "\ttest reset" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Reset);
    if(!ConfimExpectedState(pPSM, STATE_NAME(InitializingState)))
        return; 
    
    // send EvInitialized, via the ICallBack interface
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);  
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvAtHome());   
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
        
    std::cout << "\ttest refreshing settings" << std::endl;
    ((ICommandTarget*)&pe)->Handle(RefreshSettings);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    // test canceling via text command
    std::cout << "\tcancel by command while exposing" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Start);
    if(!ConfimExpectedState(pPSM, STATE_NAME(PrintSetupState)))
        return; 
    
    GoToStartPosition(&pe);  
    
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    ((ICommandTarget*)&pe)->Handle(Cancel);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvAtHome());
    pPSM->process_event(EvStartPrint());
    
    GoToStartPosition(&pe);
    
    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;        
    
    pPSM->process_event(EvExposed());
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;     
    
    std::cout << "\tcancel by command while separating" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Cancel);
    // doesn't take effect till we leave Separating
    if(!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;
    
    status = '0';
    ((ICallback*)&pe)->Callback(RotationInterrupt, &status);
    
    pPSM->process_event(EvSeparated());
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\tcancel by command while moving to start position" << std::endl;
    pPSM->process_event(EvAtHome());
    pPSM->process_event(EvStartPrint());
    
    GoToStartPosition(&pe);
    
    status = SUCCESS;
    ((ICommandTarget*)&pe)->Handle(Cancel);
    // doesn't take effect till we enter exposing
    if(!ConfimExpectedState(pPSM, STATE_NAME(MovingToStartPositionState)))
        return;  
    
    // send EvAtStartPosition, via the ICallback interface
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;   
    
    pPSM->process_event(EvAtHome());   
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    std::cout << "\tresume from ConfirmCancel, when cancel wasn't requested from Paused" << std::endl;
    pPSM->process_event(EvStartPrint());
    
    GoToStartPosition(&pe);

    status = SUCCESS;
    ((ICallback*)&pe)->Callback(MotorInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;        
    
    // request cancel while exposing
    status = BTN1_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 
    
    // but don't confirm it
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    
    std::cout << "\ttest layer thickness setting too large" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Cancel);    
    pPSM->process_event(EvAtHome());
    SETTINGS.Set(LAYER_THICKNESS, 10000);
    pPSM->process_event(EvStartPrint());
    if(!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 
    
    // go back home
    pPSM->process_event(EvRightButton()); 
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvAtHome());   
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 

    //////////////////////////////////////////////////////////
    // testing clearing print data should only be done once it's no longer 
    // needed by other tests
    //////////////////////////////////////////////////////////
 
    // verify print data exists
    if(!pe.HasAtLeastOneLayer())
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=missing print data" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // set strings that should be cleared when print data is cleared
    SETTINGS.Set(JOB_NAME_SETTING, std::string("Good job!"));
    SETTINGS.Set(JOB_ID_SETTING, std::string("My ID"));
    SETTINGS.Set(PRINT_FILE_SETTING, std::string("last chance.tar.gz"));
    
    if(SETTINGS.GetString(JOB_NAME_SETTING).empty() ||
       SETTINGS.GetString(JOB_ID_SETTING).empty()   ||
       SETTINGS.GetString(PRINT_FILE_SETTING).empty()) 
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=settings not set before clearing print data" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }

    std::cout << "\tabout to clear print data via left button press" << std::endl;
    status = BTN1_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;
    
    // verify print data cleared
    if(pe.HasAtLeastOneLayer())
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=print data not cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    std::cout << "\ton right button press when no print data, stay Home" << std::endl;
    status = BTN2_PRESS;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, &status);
    if(!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;    

    // verify job name, and ID, and last file downloaded all cleared
    if(!SETTINGS.GetString(JOB_NAME_SETTING).empty() ||
       !SETTINGS.GetString(JOB_ID_SETTING).empty()   ||
       !SETTINGS.GetString(PRINT_FILE_SETTING).empty())    
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=settings not cleared when print data cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    std::cout << "\ttest completed" << std::endl;
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

    return (mainReturnValue);
}

