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
// and UI sub-state
std::string SparkStatus::GetSparkStatus(PrintEngineState state, 
                                              UISubState substate)
{
    static bool initialized = false;

    if (!initialized)
    {
        // initialize the map of Spark states
        _stateMap[PS_KEY(HomeState, NoUISubState)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, Registered)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, NoPrintData)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, DownloadingPrintData)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomeState, PrintDownloadFailed)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, LoadingPrintData)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomeState, LoadedPrintData)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, HavePrintData)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, PrintDataLoadFailed)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, WiFiConnecting)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomeState, WiFiConnectionFailed)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, WiFiConnected)] = SPARK_READY;
        _stateMap[PS_KEY(HomeState, USBDriveFileFound)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomeState, USBDriveError)] = SPARK_BUSY;
        
        _stateMap[PS_KEY(MovingToStartPositionState, CalibratePrompt)] = 
                                                                 SPARK_PRINTING;
        _stateMap[PS_KEY(MovingToStartPositionState, NoUISubState)] = 
                                                                 SPARK_PRINTING;
        _stateMap[PS_KEY(PrintingLayerState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(PressingState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(PressingState, AboutToPause)] = SPARK_BUSY;        
        _stateMap[PS_KEY(PressDelayState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(PressDelayState, AboutToPause)] = SPARK_BUSY;        
        _stateMap[PS_KEY(UnpressingState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(UnpressingState, AboutToPause)] = SPARK_BUSY;        
        _stateMap[PS_KEY(SeparatingState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(SeparatingState, AboutToPause)] = SPARK_BUSY;        
        _stateMap[PS_KEY(ApproachingState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(ApproachingState, AboutToPause)] = SPARK_BUSY;
        _stateMap[PS_KEY(ExposingState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(ExposingState, AboutToPause)] = SPARK_BUSY;
        _stateMap[PS_KEY(PreExposureDelayState, NoUISubState)] = SPARK_PRINTING;
        _stateMap[PS_KEY(PreExposureDelayState, AboutToPause)] = SPARK_BUSY;
        _stateMap[PS_KEY(MovingToPauseState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(MovingToResumeState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(GettingFeedbackState, NoUISubState)] = SPARK_BUSY;
        
        _stateMap[PS_KEY(PausedState, NoUISubState)] = SPARK_PAUSED;
        _stateMap[PS_KEY(ConfirmCancelState, NoUISubState)] = SPARK_PAUSED;
        _stateMap[PS_KEY(AwaitingCancelationState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(UnjammingState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(JammedState, NoUISubState)] = SPARK_PAUSED;
        
        _stateMap[PS_KEY(HomingState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomingState, PrintCompleted)] = SPARK_BUSY;
        _stateMap[PS_KEY(HomingState, PrintCanceled)] = SPARK_BUSY;

        _stateMap[PS_KEY(DoorOpenState, NoUISubState)] = SPARK_MAINTENANCE;
        _stateMap[PS_KEY(DoorOpenState, ExitingDoorOpen)] = SPARK_BUSY;
        
        _stateMap[PS_KEY(DoorClosedState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(PrinterOnState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(InitializingState, NoUISubState)] = SPARK_BUSY;
        
        _stateMap[PS_KEY(ErrorState, NoUISubState)] = SPARK_ERROR;
        _stateMap[PS_KEY(ShowingVersionState, NoUISubState)] = SPARK_BUSY;
        _stateMap[PS_KEY(RegisteringState, NoUISubState)] = SPARK_BUSY;
        
        _stateMap[PS_KEY(CalibratingState, NoUISubState)] = SPARK_BUSY;;
        _stateMap[PS_KEY(DemoModeState, NoUISubState)] = SPARK_BUSY;;
     
        initialized = true;
    }
    
    if (!Validate(state, substate))
        return "";
    
    // make sure the given key exists in the map
    PrinterStatusKey psKey = PS_KEY(state, substate);
    if (_stateMap.count(psKey) < 1)
    {
        LOGGER.HandleError(UnknownSparkStatus, false, NULL, 
                                                      PS_KEY(state, substate));
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
        _jobStateMap[PS_KEY(HomeState, NoUISubState)] =      SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, Registered)] =        SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, NoPrintData)] =           SPARK_JOB_NONE;
        _jobStateMap[PS_KEY(HomeState, DownloadingPrintData)] =  SPARK_JOB_NONE;
        _jobStateMap[PS_KEY(HomeState, PrintDownloadFailed)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, LoadingPrintData)] =      SPARK_JOB_NONE;
        _jobStateMap[PS_KEY(HomeState, LoadedPrintData)] = SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, HavePrintData)] =     SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, PrintDataLoadFailed)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, WiFiConnecting)] =    SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, WiFiConnectionFailed)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, WiFiConnected)] =     SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, USBDriveFileFound)] = SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomeState, USBDriveError)] =     SPARK_JOB_RECEIVED;

        
        _jobStateMap[PS_KEY(MovingToStartPositionState, CalibratePrompt)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(MovingToStartPositionState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PrintingLayerState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PressingState, NoUISubState)] = SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PressingState, AboutToPause)] = SPARK_JOB_PRINTING;        
        _jobStateMap[PS_KEY(PressDelayState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PressDelayState, AboutToPause)] = 
                                                             SPARK_JOB_PRINTING;        
        _jobStateMap[PS_KEY(UnpressingState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(UnpressingState, AboutToPause)] = 
                                                             SPARK_JOB_PRINTING;          
        _jobStateMap[PS_KEY(SeparatingState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(SeparatingState, AboutToPause)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(ApproachingState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(ApproachingState, AboutToPause)] = 
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(ExposingState, NoUISubState)] =  SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(ExposingState, AboutToPause)] =  SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PreExposureDelayState, NoUISubState)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(PreExposureDelayState, AboutToPause)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(MovingToPauseState, NoUISubState)] =  
                                                             SPARK_JOB_PRINTING;
        _jobStateMap[PS_KEY(MovingToResumeState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        
        _jobStateMap[PS_KEY(PausedState, NoUISubState)] =      SPARK_JOB_PAUSED;
        _jobStateMap[PS_KEY(ConfirmCancelState, NoUISubState)] = 
                                                               SPARK_JOB_PAUSED;
        _jobStateMap[PS_KEY(AwaitingCancelationState, NoUISubState)] = 
                                                             SPARK_JOB_CANCELED;

        _jobStateMap[PS_KEY(UnjammingState, NoUISubState)] =   SPARK_JOB_PAUSED;
        _jobStateMap[PS_KEY(JammedState, NoUISubState)] =      SPARK_JOB_PAUSED;
        
        _jobStateMap[PS_KEY(HomingState, NoUISubState)] =    SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(HomingState, PrintCompleted)] = SPARK_JOB_COMPLETED;
        _jobStateMap[PS_KEY(HomingState, PrintCanceled)] =   SPARK_JOB_CANCELED;
       
        _jobStateMap[PS_KEY(GettingFeedbackState, NoUISubState)] =   
                                                            SPARK_JOB_COMPLETED;

        _jobStateMap[PS_KEY(PrinterOnState, NoUISubState)] = SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(InitializingState, NoUISubState)] = 
                                                             SPARK_JOB_RECEIVED;

        _jobStateMap[PS_KEY(ShowingVersionState, NoUISubState)] = 
                                                             SPARK_JOB_RECEIVED;
        _jobStateMap[PS_KEY(RegisteringState, NoUISubState)] = 
                                                             SPARK_JOB_RECEIVED;
        
        _jobStateMap[PS_KEY(CalibratingState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;        
        
        _jobStateMap[PS_KEY(DemoModeState, NoUISubState)] = SPARK_JOB_NONE;
        
        // if we're not printing, all these job states will just be 'received' 
        _jobStateMap[PS_KEY(DoorOpenState, NoUISubState)] =  SPARK_JOB_PRINTING;
        _specialKeys[PS_KEY(DoorOpenState, NoUISubState)] =  SPARK_JOB_RECEIVED;
        
        _jobStateMap[PS_KEY(DoorOpenState, ExitingDoorOpen)] = 
                                                             SPARK_JOB_PRINTING;     
        _specialKeys[PS_KEY(DoorOpenState, ExitingDoorOpen)] =  
                                                             SPARK_JOB_RECEIVED;

        _jobStateMap[PS_KEY(DoorClosedState, NoUISubState)] = 
                                                             SPARK_JOB_PRINTING;
        _specialKeys[PS_KEY(DoorClosedState, NoUISubState)] =  
                                                             SPARK_JOB_RECEIVED;

        _jobStateMap[PS_KEY(ErrorState, NoUISubState)] = SPARK_JOB_FAILED; 
        _specialKeys[PS_KEY(ErrorState, NoUISubState)] =  SPARK_JOB_RECEIVED;
     
        initialized = true;
    }
    
    // if there's no printable data, there's no job that can have any status
    // the print file setting always accompanies print data and thus determines
    // the presence of printable data
    if (SETTINGS.GetString(PRINT_FILE_SETTING).empty())
        return SPARK_JOB_NONE;
    
    if (!Validate(state, substate))
        return "";
    
    // make sure the given key exists in the map
    PrinterStatusKey psKey = PS_KEY(state, substate);
    if (_jobStateMap.count(psKey) < 1)
    {
        LOGGER.HandleError(UnknownSparkJobStatus, false, NULL, 
                                                      PS_KEY(state, substate));
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
        LOGGER.HandleError(UnknownPrintEngineState, false, NULL, state);
        retVal = false;                                                              
    }
    else if (substate < NoUISubState || substate >= MaxUISubState)
    {
        LOGGER.HandleError(UnknownUISubState, false, NULL, substate);
        retVal = false;                                                                
    }
    
    return retVal;
}
