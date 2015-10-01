//  File:   PrintEngineUT.cpp
//  Tests PrintEngine and its state machine
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <iostream>
#include <PrintEngine.h>
#include <PrinterStateMachine.h>
#include <Settings.h>
#include <stdio.h>
#include <string.h>
#include <Hardware.h>
#include <Shared.h>
#include <MotorController.h>
#include <utils.h>
#include <sstream>
#include <sys/stat.h>
#include <Filenames.h>

#include "support/FileUtils.hpp"
#include "PrinterStatusQueue.h"
#include "Motor.h"
#include "Timer.h"

int mainReturnValue = EXIT_SUCCESS;
std::string testPrintDataDir, testStagingDir, testDownloadDir, testPerLayerSettingsFile;

void Setup()
{
    // Create a temp directories
    testPrintDataDir = CreateTempDir();
    testStagingDir = CreateTempDir();
    testDownloadDir = CreateTempDir();
    
    // backup the current smith_state and settings files
    rename(SMITH_STATE_FILE, "smith_state_backup");
    rename(SETTINGS_PATH, "settings_backup");
    // and use one that indicates Internet connected, 
    // for testing GettingFeedback state
    Copy("resources/smith_state_connected", SMITH_STATE_FILE);
   
    // Restore all to ensure known initial conditions
    SETTINGS.RestoreAll();
    
    // Update settings with test directory paths
    SETTINGS.Set(PRINT_DATA_DIR, testPrintDataDir);
    SETTINGS.Set(DOWNLOAD_DIR, testDownloadDir);
    SETTINGS.Set(STAGING_DIR, testStagingDir);

    // put data in place for PrintDataDirectory
    std::ostringstream ss;
    ss << testPrintDataDir + "/" + PRINT_DATA_NAME;
    std::string testPrintDataSubdirectory = ss.str();
    mkdir(testPrintDataSubdirectory.c_str(), 0755);
    Copy("resources/slices/slice_1.png", testPrintDataSubdirectory + "/slice_1.png");
    Copy("resources/slices/slice_2.png", testPrintDataSubdirectory + "/slice_2.png");
    Copy("resources/slices/slice_2.png", testPrintDataSubdirectory + "/slice_3.png");
    // include per-layer settings file with overpress settings
    testPerLayerSettingsFile = testPrintDataSubdirectory + "/" + PER_LAYER_SETTINGS_FILE;
    Copy("resources/print_engine_ut_layer_params.csv", testPerLayerSettingsFile);
    
    // set the HW rev to test jamming detection 
    SETTINGS.Set(HARDWARE_REV, 1);
    SETTINGS.Set(MAX_UNJAM_TRIES, 2); 
    SETTINGS.Set(DETECT_JAMS, 1); 

    // set pre-exposure delays to non-zero values, so that EvDelayEnd event will be needed 
    SETTINGS.Set(FL_APPROACH_WAIT, 1000);
    SETTINGS.Set(BI_APPROACH_WAIT, 1000);
    SETTINGS.Set(ML_APPROACH_WAIT, 1000);   
    SETTINGS.Set(BURN_IN_LAYERS, 1);
    SETTINGS.Set(FL_PRESS, 0);  // disabled for first layer
    SETTINGS.Set(BI_PRESS, 1000);
    SETTINGS.Set(ML_PRESS, 0);  // non-zero value contained in per-layer settings
    SETTINGS.Set(FL_PRESS_WAIT, 0);
    SETTINGS.Set(BI_PRESS_WAIT, 0); // no delay for 2nd (burn-in) layer
    SETTINGS.Set(ML_PRESS_WAIT, 1000);

    // save settings
    SETTINGS.Save();
}

void TearDown()
{
    // restore the original smith_state file
    rename("smith_state_backup", SMITH_STATE_FILE);
    rename("settings_backup", SETTINGS_PATH);
 
    RemoveDir(testPrintDataDir);
    RemoveDir(testStagingDir);
    RemoveDir(testDownloadDir);
}

// method to determine if we're in the expected state
// Note: it doesn't work for orthogonal states
bool ConfimExpectedState(const PrinterStateMachine* pPSM , const char* expected, bool fail = true)
{   
    const char* name;
    
    for (PrinterStateMachine::state_iterator pLeafState = pPSM->state_begin();
         pLeafState != pPSM->state_end(); ++pLeafState)
    {
        name = typeid(*pLeafState).name();
        if (strstr(name, expected) != NULL)
            return true;
    }
    if (fail)
    {
        // here we must not have found a match, in any orthogonal region
        std::cout << "expected " << expected << " but actual state was " 
                                 << name << std::endl;
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=unexpected state" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    return false;
}

void DisplayStateConfiguration(const PrinterStateMachine* pPSM)
{
  printf("\t\tstate config = ");
  char region = 'a';

  for (
    PrinterStateMachine::state_iterator pLeafState = pPSM->state_begin();
    pLeafState != pPSM->state_end(); ++pLeafState)
  {
    std::cout << "Orthogonal region " << region << ": ";
   // std::cout << pLeafState->custom_dynamic_type_ptr< char >() << "\n";
    std::cout << typeid(*pLeafState).name() << "\n";
    ++region;
  }
  
  std::cout << "" << std::endl;
}

void test1() {  
    unsigned char button1Press = BTN1_PRESS;
    unsigned char button2Press = BTN2_PRESS;
    unsigned char button1Hold = BTN1_HOLD;
    unsigned char mcSuccess = MC_STATUS_SUCCESS;
    unsigned char mcError = 0xFF;
    char gpioHigh = '1';
    char gpioLow = '0';
    
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    std::cout << "\tabout to instantiate & initiate printer" << std::endl;
    
    // don't require use of real hardware
    Motor motor(0xFF); // 0xFF results in "null" I2C device that does not actually write to the bus
    PrinterStatusQueue printerStatusQueue;
    Timer timer1;
    Timer timer2;
    Timer timer3;
    Timer timer4;
    PrintEngine pe(false, motor, printerStatusQueue, timer1, timer2, timer3, timer4);
    pe.Begin();
    
    PrinterStateMachine* pPSM = pe.GetStateMachine();
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;

    std::cout << "\tabout to process reset event" << std::endl;
    pPSM->process_event(EvReset());
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;    
    
    std::cout << "\tabout to process door opened event" << std::endl;
    ((ICallback*)&pe)->Callback(DoorInterrupt, EventData(gpioHigh)); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    ((ICallback*)&pe)->Callback(DoorInterrupt, EventData(gpioLow)); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;     
    
    std::cout << "\tabout to process door opened event again" << std::endl;
    pPSM->process_event(EvDoorOpened());
    if (!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;   

    std::cout << "\tabout to process reset event again" << std::endl;
    pPSM->process_event(EvReset());
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;   
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;   

    std::cout << "\tabout to process show version event" << std::endl;
    pPSM->process_event(EvLeftButtonHold());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ShowingVersionState)))
        return;

    std::cout << "\tabout to process hide version event" << std::endl;
    pPSM->process_event(EvRightButton()); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;  
    
    
    std::cout << "\tabout to test registration" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(StartRegistering);
    if (!ConfimExpectedState(pPSM, STATE_NAME(RegisteringState)))
        return; 
    
    ((ICommandTarget*)&pe)->Handle(RegistrationSucceeded);
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button2Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 

    std::cout << "\tabout to test main path" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Start);
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToStartPositionState)))
        return; 
        
    std::cout << "\tabout to start printing" << std::endl;
    // send EvAtStartPosition, via the ICallback interface
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(CalibratingState)))
        return;         
    
    // indicate calibration done
    pPSM->process_event(EvRightButton());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return;        
    
    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    std::cout << "\tabout to process door opened event" << std::endl;
    pPSM->process_event(EvDoorOpened()); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;

    std::cout << "\tabout to process door closed event" << std::endl;    
    pPSM->process_event(EvDoorClosed());    
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
        
    std::cout << "\tabout to recover from resin tray jam" << std::endl;
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnjammingState)))
        return; 
     
    // first attempt to unjam fails
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnjammingState)))
        return; 
    
    // but second attempt frees the jam
    ((ICallback*)&pe)->Callback(RotationInterrupt, EventData(gpioLow));
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PressingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnpressingState)))
        return;     
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return; 
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    std::cout << "\tabout to handle unrecoverable resin tray jam" << std::endl;
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnjammingState)))
        return; 
    
    // try unjamming once
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnjammingState)))
        return; 

    // try unjamming second time, unsuccessfully
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(JammedState)))
        return; 
    
    // ask to cancel, but then resume, after jam (to very fix for defect DE33))
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button1Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return;
     
    pPSM->process_event(EvResume());       
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 
    
    // overpress without delay for 2nd (Burn-In) layer)
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PressingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnpressingState)))
        return;     
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
   
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;  
    
    // this time provide rotation interrupt while separating
    ((ICallback*)&pe)->Callback(RotationInterrupt, EventData(gpioLow)); 
    // but then open and close the door (to very fix for defect DE32) 
    pPSM->process_event(EvDoorOpened()); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(DoorOpenState)))
        return;
 
    pPSM->process_event(EvDoorClosed());    
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;
   
    // allow the print to complete with a 3rd (Model) layer
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 

    // do tray deflection with delay
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PressingState)))
        return; 

    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PressDelayState)))
        return; 
    
    pe.ClearDelayTimer();
    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnpressingState)))
        return;  
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 
    
    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
   
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;  
    
    ((ICallback*)&pe)->Callback(RotationInterrupt, EventData(gpioLow)); 
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 

    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(GettingFeedbackState)))
        return;     
    
    // dismiss the feedback screen
    ((ICommandTarget*)&pe)->Handle(Dismiss);
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 

    std::cout << "\tabout to process an error" << std::endl;
    pPSM->process_event(EvError());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 

    // reset & get back to where we can test pause/resume
    std::cout << "\tabout to start printing again" << std::endl;
    //test button event commands
    ((ICommandTarget*)&pe)->Handle(Button1);
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());

    // remove per-layer settings file to verify that PrintEngine
    // clears per-layer settings from previous print
    remove(testPerLayerSettingsFile.c_str());
    
    // now needs second start command
    pPSM->process_event(EvStartPrint());
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToStartPositionState)))
        return;  
     
    // skip calibration, via simulated button action
    ((ICommandTarget*)&pe)->Handle(Button2);

    
    // send EvAtStartPosition, via the ICallback interface
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    pe.ClearExposureTimer();
    ((ICallback*)&pe)->Callback(ExposureEnd, EventData(1));
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 

    // test pause/resume
    std::cout << "\tabout to pause" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Pause);
    // requesting a pause while separating just sets a flag
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
    
    // send EvSeparated, & EvApproached via the ICallback interface, 
    // with rotation interrupt,
    // which should now start the pause & inspect process
    ((ICallback*)&pe)->Callback(RotationInterrupt, EventData(gpioLow));
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToPauseState)))
        return; 

    // complete pausing 
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PausedState)))
        return; 

    std::cout << "\tabout to resume" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Resume);
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToResumeState)))
        return; 

    // set insufficient headroom for lifting to inspection position
    SETTINGS.Set(MAX_Z_TRAVEL, 0);

    // overpress without delay for 2nd (Burn-In) layer)
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(PressingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(UnpressingState)))
        return;     
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;  

    std::cout << "\tabout to pause using right button" << std::endl; 
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button2Press));
    // requesting a pause while separating also just sets a flag
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return; 
        
    ((ICallback*)&pe)->Callback(RotationInterrupt, EventData(gpioLow));
    pPSM->process_event(EvMotionCompleted());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ApproachingState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    // even though there's no room to lift for inspection, we still rotate to
    // the paused position
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToPauseState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PausedState)))
        return; 
    
    std::cout << "\tabout to request cancel" << std::endl;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button1Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 
    
    std::cout << "\tbut not confirm cancel" << std::endl;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button2Press));  
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToResumeState)))
        return; 
    
    // allow the print to complete with a 3rd (Model) layer
    // above, we deleted the per-layer settings file that previously caused the
    // overpress for the third layer
    // verify that the PrintEngine cleared the per-layer settings by
    // expecting not to go through overpress
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pe.ClearExposureTimer();
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;  
    
    std::cout << "\tabout to request cancel again" << std::endl;
    pPSM->process_event(EvLeftButton());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 

    std::cout << "\twith confirmation this time" << std::endl;
    pPSM->process_event(EvLeftButton());
    if (!ConfimExpectedState(pPSM, STATE_NAME(AwaitingCancelationState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
     if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\thandle a non-fatal error" << std::endl;
    ((ICallback*)&pe)->Callback(PrinterStatusUpdate, EventData(PrinterStatus()));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;     

    std::cout << "\thandle a fatal error" << std::endl;
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcError));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 
    
    std::cout << "\tabout to process show version event via left button hold" << std::endl;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button1Hold));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ShowingVersionState)))
        return;

    std::cout << "\tabout to process hide version event again" << std::endl;
    pPSM->process_event(EvRightButton()); 
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return;

    // reset
    pPSM->process_event(EvLeftButton());   
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\thandle another fatal error" << std::endl;
    ((ICallback*)&pe)->Callback(MotorTimeout, EventData(mcError));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return; 
    
    std::cout << "\ttest reset command" << std::endl;
    ((ICommandTarget*)&pe)->Handle(Reset);
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());   
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
        
    std::cout << "\ttest refreshing settings" << std::endl;
    ((ICommandTarget*)&pe)->Handle(RefreshSettings);
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    // test canceling via text command
    std::cout << "\tcancel by command while exposing" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Start);
    if (!ConfimExpectedState(pPSM, STATE_NAME(MovingToStartPositionState)))
        return;  
        
    // skip calibration
    pPSM->process_event(EvRightButton());
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;    
    
    ((ICommandTarget*)&pe)->Handle(Cancel);
    if (!ConfimExpectedState(pPSM, STATE_NAME(AwaitingCancelationState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));    
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    pPSM->process_event(EvMotionCompleted());
    pPSM->process_event(EvStartPrint());
    
    // skip calibration
    pPSM->process_event(EvRightButton());
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;        
    
    pPSM->process_event(EvExposed());
    if (!ConfimExpectedState(pPSM, STATE_NAME(SeparatingState)))
        return;     
    
    std::cout << "\tcancel by command while separating" << std::endl; 
    ((ICommandTarget*)&pe)->Handle(Cancel);
    if (!ConfimExpectedState(pPSM, STATE_NAME(AwaitingCancelationState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return; 
    
    std::cout << "\tcancel by command while moving to start position" << std::endl;
    pPSM->process_event(EvMotionCompleted());
    pPSM->process_event(EvStartPrint());
    
    // skip calibration
    pPSM->process_event(EvRightButton());
    
    ((ICommandTarget*)&pe)->Handle(Cancel);
    if (!ConfimExpectedState(pPSM, STATE_NAME(AwaitingCancelationState)))
        return; 
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomingState)))
        return;   
    
    pPSM->process_event(EvMotionCompleted());   
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    std::cout << "\tresume from ConfirmCancel, when cancel wasn't requested from Paused" << std::endl;
    pPSM->process_event(EvStartPrint());

    // skip calibration
    pPSM->process_event(EvRightButton());
    
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    if (!ConfimExpectedState(pPSM, STATE_NAME(PreExposureDelayState)))
        return; 

    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;        
    
    // request cancel while exposing
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button1Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ConfirmCancelState)))
        return; 
    
    // but don't confirm it
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button2Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    ((ICommandTarget*)&pe)->Handle(Cancel);    
    pPSM->process_event(EvMotionCompleted());   // get out of AwaiingCancelation
    pPSM->process_event(EvMotionCompleted());   // get out of Homing
    
    
    std::cout << "\ttest error handling while printing" << std::endl;
    // set separation speed to illegal value of 0
    SETTINGS.Set(FL_SEPARATION_R_SPEED, 0);
    pPSM->process_event(EvStartPrint());
    // skip calibration
    pPSM->process_event(EvRightButton());
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pPSM->process_event(EvExposed());
    // here, instead of getting to Separating state, 
    // we should get to the ErrorState
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return;

    // reset
    pPSM->process_event(EvLeftButton());   
    pPSM->process_event(EvMotionCompleted());   
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 
    
    // set separation speed to illegal value of -1
    SETTINGS.Set(FL_SEPARATION_R_SPEED, -1);
    pPSM->process_event(EvStartPrint());
    // skip calibration
    pPSM->process_event(EvRightButton());
    ((ICallback*)&pe)->Callback(MotorInterrupt, EventData(mcSuccess));
    pPSM->process_event(EvDelayEnded());
    if (!ConfimExpectedState(pPSM, STATE_NAME(ExposingState)))
        return;
    
    pPSM->process_event(EvExposed());
    // here, instead of getting to Separating state, 
    // we should get to the ErrorState
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return;

    // reset and return home
    pPSM->process_event(EvLeftButton());   
    pPSM->process_event(EvMotionCompleted());   
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return; 

    //////////////////////////////////////////////////////////
    // testing clearing print data should only be done once it's no longer 
    // needed by other tests
    //////////////////////////////////////////////////////////
 
    // verify print data exists
    if (!pe.HasAtLeastOneLayer())
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=missing print data" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // set strings that should be cleared when print data is cleared
    SETTINGS.Set(JOB_NAME_SETTING, std::string("Good job!"));
    SETTINGS.Set(JOB_ID_SETTING, std::string("My ID"));
    SETTINGS.Set(PRINT_FILE_SETTING, std::string("last chance.tar.gz"));
    
    if (SETTINGS.GetString(JOB_NAME_SETTING).empty() ||
       SETTINGS.GetString(JOB_ID_SETTING).empty()   ||
       SETTINGS.GetString(PRINT_FILE_SETTING).empty()) 
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=settings not set before clearing print data" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }

    std::cout << "\tabout to clear print data via left button press" << std::endl;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button1Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;
    
    // verify print data cleared
    if (pe.HasAtLeastOneLayer())
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=print data not cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    std::cout << "\ton right button press when no print data, stay Home" << std::endl;
    ((ICallback*)&pe)->Callback(ButtonInterrupt, EventData(button2Press));
    if (!ConfimExpectedState(pPSM, STATE_NAME(HomeState)))
        return;    

    // verify job name, and ID, and last file downloaded all cleared
    if (!SETTINGS.GetString(JOB_NAME_SETTING).empty() ||
       !SETTINGS.GetString(JOB_ID_SETTING).empty()   ||
       !SETTINGS.GetString(PRINT_FILE_SETTING).empty())    
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=settings not cleared when print data cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
  
    // PrintEngine applies settings from temp file when handling ApplySettings
    remove(TEMP_SETTINGS_FILE);
    Copy("resources/good_settings", TEMP_SETTINGS_FILE);
    ((ICommandTarget*)&pe)->Handle(ApplySettings);
    std::string actualJobName = SETTINGS.GetString(JOB_NAME_SETTING);
    std::string expectedJobName = "NewJobName";
    if (actualJobName != expectedJobName)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=print settings from temp file not applied after handling ApplySettings; expected job name to equal \"" << expectedJobName << "\", got \"" << actualJobName << "\"" << std::endl;
        mainReturnValue = EXIT_FAILURE;

    }
   
    // PrintEngine removes temp settings file when handling ApplySettings
    std::ifstream tempSettingsFile(TEMP_SETTINGS_FILE);
    if (tempSettingsFile.good())
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (PrintEngineUT) message=temp settings file not removed after handling ApplySettings" << std::endl;
        remove(TEMP_SETTINGS_FILE);
        mainReturnValue = EXIT_FAILURE;
    }

    // test error handling when temp settings file does not exist when handling ApplySettings
    remove(TEMP_SETTINGS_FILE);
    ((ICommandTarget*)&pe)->Handle(ApplySettings);
    if (!ConfimExpectedState(pPSM, STATE_NAME(ErrorState)))
        return;
    
    // reset and return home
    pPSM->process_event(EvLeftButton());
    pPSM->process_event(EvMotionCompleted());

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

