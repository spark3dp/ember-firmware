//  File:   SparkStatus.cpp
//  Defines the states recognized by the Spark API, as a function of 
//  PrintEngineState and UISubState.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#include <SparkStatus.h>
#include <Logger.h>
#include <Shared.h>
#include <Settings.h>

std::map<PrinterStatusKey, std::string> SparkStatus::_stateMap;

// Gets the Spark API printer state based on the PrintEngine state 
// and UI sub-state.  If canLoadPrintData is true, the Spark state is "ready",
// regardless of the PrintEngine state and sub-state.
std::string SparkStatus::GetSparkStatus(PrintEngineState state, 
                                        UISubState substate, 
                                        bool canLoadPrintData)
{
    static bool initialized = false;

    if (!initialized)
    {
        // initialize the map of Spark states
        _stateMap[Key(HomeState, NoUISubState)] =                SPARK_READY;
        _stateMap[Key(HomeState, Registered)] =                  SPARK_READY;
        _stateMap[Key(HomeState, NoPrintData)] =                 SPARK_READY;
        _stateMap[Key(HomeState, DownloadingPrintData)] =        SPARK_BUSY;
        _stateMap[Key(HomeState, PrintDownloadFailed)] =         SPARK_READY;
        _stateMap[Key(HomeState, LoadingPrintData)] =            SPARK_BUSY;
        _stateMap[Key(HomeState, LoadedPrintData)] =             SPARK_READY;
        _stateMap[Key(HomeState, HavePrintData)] =               SPARK_READY;
        _stateMap[Key(HomeState, PrintDataLoadFailed)] =         SPARK_READY;
        _stateMap[Key(HomeState, WiFiConnecting)] =              SPARK_BUSY;
        _stateMap[Key(HomeState, WiFiConnectionFailed)] =        SPARK_READY;
        _stateMap[Key(HomeState, WiFiConnected)] =               SPARK_READY;
        _stateMap[Key(HomeState, USBDriveFileFound)] =           SPARK_BUSY;
        _stateMap[Key(HomeState, USBDriveError)] =               SPARK_BUSY;
        
        _stateMap[Key(MovingToStartPositionState, CalibratePrompt)] = 
                                                                 SPARK_PRINTING;
        _stateMap[Key(MovingToStartPositionState, NoUISubState)] = 
                                                                 SPARK_PRINTING;
        _stateMap[Key(PrintingLayerState, NoUISubState)] =       SPARK_PRINTING;
        _stateMap[Key(InitializingLayerState, NoUISubState)] =   SPARK_PRINTING;
        _stateMap[Key(InitializingLayerState, AboutToPause)] =   SPARK_BUSY;
        _stateMap[Key(PressingState, NoUISubState)] =            SPARK_PRINTING;
        _stateMap[Key(PressingState, AboutToPause)] =            SPARK_BUSY;        
        _stateMap[Key(PressDelayState, NoUISubState)] =          SPARK_PRINTING;
        _stateMap[Key(PressDelayState, AboutToPause)] =          SPARK_BUSY;        
        _stateMap[Key(UnpressingState, NoUISubState)] =          SPARK_PRINTING;
        _stateMap[Key(UnpressingState, AboutToPause)] =          SPARK_BUSY;        
        _stateMap[Key(SeparatingState, NoUISubState)] =          SPARK_PRINTING;
        _stateMap[Key(SeparatingState, AboutToPause)] =          SPARK_BUSY;        
        _stateMap[Key(ApproachingState, NoUISubState)] =         SPARK_PRINTING;
        _stateMap[Key(ApproachingState, AboutToPause)] =         SPARK_BUSY;
        _stateMap[Key(ExposingState, NoUISubState)] =            SPARK_PRINTING;
        _stateMap[Key(ExposingState, AboutToPause)] =            SPARK_BUSY;
        _stateMap[Key(PreExposureDelayState, NoUISubState)] =    SPARK_PRINTING;
        _stateMap[Key(PreExposureDelayState, AboutToPause)] =    SPARK_BUSY;
        _stateMap[Key(MovingToPauseState, NoUISubState)] =       SPARK_BUSY;
        _stateMap[Key(MovingToResumeState, NoUISubState)] =      SPARK_BUSY;
        _stateMap[Key(GettingFeedbackState, NoUISubState)] =     SPARK_BUSY;
        
        _stateMap[Key(PausedState, NoUISubState)] = SPARK_PAUSED;
        _stateMap[Key(ConfirmCancelState, NoUISubState)] =       SPARK_PAUSED;
        _stateMap[Key(AwaitingCancelationState, NoUISubState)] = SPARK_BUSY;
        _stateMap[Key(UnjammingState, NoUISubState)] =           SPARK_BUSY;
        _stateMap[Key(JammedState, NoUISubState)] =              SPARK_PAUSED;
        
        _stateMap[Key(HomingState, NoUISubState)] =              SPARK_BUSY;
        _stateMap[Key(HomingState, PrintCompleted)] =            SPARK_BUSY;
        _stateMap[Key(HomingState, PrintCanceled)] =             SPARK_BUSY;

        // the Spark status for DoorOpenState, NoUISubState is overridden as
        // SPARK_READY when canLoadPrintData is true
        _stateMap[Key(DoorOpenState, NoUISubState)] =         SPARK_MAINTENANCE;
        _stateMap[Key(DoorOpenState, ClearingScreen)] =         SPARK_BUSY;
        _stateMap[Key(DoorOpenState, LoadedPrintData)] =         SPARK_READY;
        _stateMap[Key(DoorOpenState, DownloadingPrintData)] =    SPARK_BUSY;
        _stateMap[Key(DoorOpenState, LoadingPrintData)] =        SPARK_BUSY;
        _stateMap[Key(DoorOpenState, PrintDataLoadFailed)] =     SPARK_READY;
        _stateMap[Key(DoorOpenState, PrintDownloadFailed)] =     SPARK_READY;
        
        _stateMap[Key(DoorClosedState, NoUISubState)] =          SPARK_BUSY;
        _stateMap[Key(PrinterOnState, NoUISubState)] =           SPARK_BUSY;
        _stateMap[Key(InitializingState, NoUISubState)] =        SPARK_BUSY;
        
        _stateMap[Key(ErrorState, NoUISubState)] =               SPARK_ERROR;
        _stateMap[Key(ShowingVersionState, NoUISubState)] =      SPARK_BUSY;
        _stateMap[Key(RegisteringState, NoUISubState)] =         SPARK_BUSY;
        
        _stateMap[Key(CalibratingState, NoUISubState)] =         SPARK_BUSY;
        _stateMap[Key(DemoModeState, NoUISubState)] =            SPARK_BUSY;
        _stateMap[Key(ConfirmUpgradeState, NoUISubState)] =      SPARK_BUSY;
        _stateMap[Key(ConfirmUpgradeState, ClearingScreen)] =    SPARK_BUSY;      
        _stateMap[Key(UpgradingProjectorState, NoUISubState)] =  SPARK_BUSY;
        _stateMap[Key(UpgradeCompleteState, NoUISubState)] =     SPARK_BUSY;
             
        initialized = true;
    }
    
    if (canLoadPrintData)
        return SPARK_READY;
    
    if (!Validate(state, substate))
        return "";
    
    // make sure the given key exists in the map
    PrinterStatusKey psKey = Key(state, substate);
    if (_stateMap.count(psKey) < 1)
    {
        Logger::HandleError(UnknownSparkStatus, false, NULL, 
                                                        Key(state, substate));
        return "";
    }
    else
        return _stateMap[psKey];
}

std::map<PrinterStatusKey, std::string> SparkStatus::_jobStateMap;
std::map<PrinterStatusKey, std::string> SparkStatus::_specialKeys;

// Gets the Spark API print job state based on the PrintEngine state 
// and UI sub-state.  For door open and error states, we also need to know 
// whether or not they happened while printing.
std::string SparkStatus::GetSparkJobStatus(PrintEngineState state, 
                                           UISubState substate, bool printing)
{
    static bool initialized = false;

    if (!initialized)
    {
        // initialize the map of Spark job states
        // note, these only apply if there is a current job, i.e. if there is
        // printable data
        _jobStateMap[Key(HomeState, NoUISubState)] =         SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, Registered)] =           SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, NoPrintData)] =          SPARK_JOB_NONE;
        _jobStateMap[Key(HomeState, DownloadingPrintData)] = SPARK_JOB_NONE;
        _jobStateMap[Key(HomeState, PrintDownloadFailed)] =  SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, LoadingPrintData)] =     SPARK_JOB_NONE;
        _jobStateMap[Key(HomeState, LoadedPrintData)] =      SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, HavePrintData)] =        SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, PrintDataLoadFailed)] =  SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, WiFiConnecting)] =       SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, WiFiConnectionFailed)] = SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, WiFiConnected)] =        SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, USBDriveFileFound)] =    SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomeState, USBDriveError)] =        SPARK_JOB_RECEIVED;

        
        _jobStateMap[Key(MovingToStartPositionState, CalibratePrompt)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(MovingToStartPositionState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(PrintingLayerState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(InitializingLayerState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(InitializingLayerState, AboutToPause)] = 
                                                             SPARK_JOB_PRINTING;        
        _jobStateMap[Key(PressingState, NoUISubState)] =     SPARK_JOB_PRINTING;
        _jobStateMap[Key(PressingState, AboutToPause)] =     SPARK_JOB_PRINTING;        
        _jobStateMap[Key(PressDelayState, NoUISubState)] =   SPARK_JOB_PRINTING;
        _jobStateMap[Key(PressDelayState, AboutToPause)] =   SPARK_JOB_PRINTING;        
        _jobStateMap[Key(UnpressingState, NoUISubState)] =   SPARK_JOB_PRINTING;
        _jobStateMap[Key(UnpressingState, AboutToPause)] =   SPARK_JOB_PRINTING;          
        _jobStateMap[Key(SeparatingState, NoUISubState)] =   SPARK_JOB_PRINTING;
        _jobStateMap[Key(SeparatingState, AboutToPause)] =   SPARK_JOB_PRINTING;
        _jobStateMap[Key(ApproachingState, NoUISubState)] =  SPARK_JOB_PRINTING;
        _jobStateMap[Key(ApproachingState, AboutToPause)] =  SPARK_JOB_PRINTING;
        _jobStateMap[Key(ExposingState, NoUISubState)] =     SPARK_JOB_PRINTING;
        _jobStateMap[Key(ExposingState, AboutToPause)] =     SPARK_JOB_PRINTING;
        _jobStateMap[Key(PreExposureDelayState, NoUISubState)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(PreExposureDelayState, AboutToPause)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(MovingToPauseState, NoUISubState)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[Key(MovingToResumeState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        
        _jobStateMap[Key(PausedState, NoUISubState)] =        SPARK_JOB_PAUSED;
        _jobStateMap[Key(ConfirmCancelState, NoUISubState)] = SPARK_JOB_PAUSED;
        _jobStateMap[Key(AwaitingCancelationState, NoUISubState)] = 
                                                             SPARK_JOB_CANCELED;

        _jobStateMap[Key(UnjammingState, NoUISubState)] =    SPARK_JOB_PAUSED;
        _jobStateMap[Key(JammedState, NoUISubState)] =       SPARK_JOB_PAUSED;
        
        _jobStateMap[Key(HomingState, NoUISubState)] =       SPARK_JOB_RECEIVED;
        _jobStateMap[Key(HomingState, PrintCompleted)] =    SPARK_JOB_COMPLETED;
        _jobStateMap[Key(HomingState, PrintCanceled)] =      SPARK_JOB_CANCELED;
       
        _jobStateMap[Key(GettingFeedbackState, NoUISubState)] =   
                                                            SPARK_JOB_COMPLETED;

        _jobStateMap[Key(PrinterOnState, NoUISubState)] =    SPARK_JOB_RECEIVED;
        _jobStateMap[Key(InitializingState, NoUISubState)] = SPARK_JOB_RECEIVED;

        _jobStateMap[Key(ShowingVersionState, NoUISubState)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[Key(RegisteringState, NoUISubState)] =  SPARK_JOB_RECEIVED;
        
        _jobStateMap[Key(CalibratingState, NoUISubState)] =  SPARK_JOB_PRINTING;        
        
        _jobStateMap[Key(DemoModeState, NoUISubState)] =         SPARK_JOB_NONE;
        _jobStateMap[Key(ConfirmUpgradeState, NoUISubState)] =   SPARK_JOB_NONE;
        _jobStateMap[Key(UpgradingProjectorState, NoUISubState)] =  
                                                                 SPARK_JOB_NONE;
        _jobStateMap[Key(ConfirmUpgradeState, ClearingScreen)] = SPARK_JOB_NONE;      

        _jobStateMap[Key(UpgradeCompleteState, NoUISubState)] =  SPARK_JOB_NONE;
           
        _jobStateMap[Key(DoorOpenState, LoadedPrintData)] =  SPARK_JOB_RECEIVED;
        _jobStateMap[Key(DoorOpenState, DownloadingPrintData)] = SPARK_JOB_NONE;
        _jobStateMap[Key(DoorOpenState, LoadingPrintData)] = SPARK_JOB_NONE;
        _jobStateMap[Key(DoorOpenState, PrintDataLoadFailed)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[Key(DoorOpenState, PrintDownloadFailed)] = 
                                                             SPARK_JOB_RECEIVED;
      
        // if we're not printing, all these job states will just be 'received' 
        _jobStateMap[Key(DoorOpenState, NoUISubState)] =     SPARK_JOB_PRINTING;
        _specialKeys[Key(DoorOpenState, NoUISubState)] =     SPARK_JOB_RECEIVED;
        
        _jobStateMap[Key(DoorOpenState, ClearingScreen)] =  SPARK_JOB_PRINTING;     
        _specialKeys[Key(DoorOpenState, ClearingScreen)] =  SPARK_JOB_RECEIVED;

        _jobStateMap[Key(DoorClosedState, NoUISubState)] =   SPARK_JOB_PRINTING;
        _specialKeys[Key(DoorClosedState, NoUISubState)] =   SPARK_JOB_RECEIVED;

        _jobStateMap[Key(ErrorState, NoUISubState)] =        SPARK_JOB_FAILED; 
        _specialKeys[Key(ErrorState, NoUISubState)] =        SPARK_JOB_RECEIVED;
     
        initialized = true;
    }
    
    // if there's no printable data, there's no job that can have any status
    // the print file setting always accompanies print data and thus determines
    // the presence of printable data
    if (PrinterSettings::Instance().GetString(PRINT_FILE_SETTING).empty())
        return SPARK_JOB_NONE;
    
    if (!Validate(state, substate))
        return "";
    
    // make sure the given key exists in the map
    PrinterStatusKey psKey = Key(state, substate);
    if (_jobStateMap.count(psKey) < 1)
    {
        Logger::HandleError(UnknownSparkJobStatus, false, NULL, 
                                                      Key(state, substate));
        return "";
    }
    else if (!printing && _specialKeys.count(psKey) > 0)
        return _specialKeys[psKey];
    else
        return _jobStateMap[psKey];
}

bool SparkStatus::Validate(PrintEngineState state, UISubState substate)
{ 
    bool retVal = true;

    if (state <= UndefinedPrintEngineState || state >= MaxPrintEngineState)
    {
        Logger::HandleError(UnknownPrintEngineState, false, NULL, state);
        retVal = false;                                                              
    }
    else if (substate < NoUISubState || substate >= MaxUISubState)
    {
        Logger::HandleError(UnknownUISubState, false, NULL, substate);
        retVal = false;                                                                
    }
    
    return retVal;
}
