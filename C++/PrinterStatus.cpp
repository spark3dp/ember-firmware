/* 
 * File:   PrinterStatus.cpp
 * Author: Richard Greene
 * 
 * The data structure used to communicate status from the print engine to UI 
 * components.
 *
 * Created on July 28, 2014, 5:25 PM
 */

#include <PrinterStatus.h>
#include <Logger.h>
#include <Shared.h>

/// Constructor
PrinterStatus::PrinterStatus() :
_state(PrinterOnState),
_change(NoChange),
_UISubState(NoUISubState),
_isError(false),
_errorCode(Success),
_errno(0),
_numLayers(0),
_currentLayer(0),
_estimatedSecondsRemaining(0),
_temperature(0.0f)
{
}

/// Gets the name of a print engine state machine state
const char* PrinterStatus::GetStateName(PrintEngineState state)
{
    static bool initialized = false;
    static const char* stateNames[MaxPrintEngineState];
    if(!initialized)
    {
        // initialize the array of state names
        stateNames[PrinterOnState] = "PrinterOn";
        stateNames[DoorClosedState] = "DoorClosed";
        stateNames[InitializingState] = "Initializing";
        stateNames[DoorOpenState] = "DoorOpen";
        stateNames[HomingState] = "Homing";
        stateNames[HomeState] = "Home";
        stateNames[IdleState] = "Idle";
        stateNames[PrintSetupState] = "PrintSetup";
        stateNames[MovingToStartPositionState] = "MovingToStartPosition";
        stateNames[ExposingState] = "Exposing";
        stateNames[PrintingState] = "Printing";
        stateNames[PrintingLayerState] = "PrintingLayer";
        stateNames[PausedState] = "Paused";
        stateNames[SeparatingState] = "Separating";
        stateNames[EndingPrintState] = "EndingPrint";  
        stateNames[ConfirmCancelState] = "ConfirmCancel";
        stateNames[ShowingVersionState] = "ShowingVersion";
        stateNames[CalibrateState] = "Calibrate";
        stateNames[MovingToCalibrationState] = "MovingToCalibration";
        stateNames[CalibratingState] = "Calibrating";
        stateNames[RegisteringState] = "Registering";
        stateNames[RegisteredState] = "Registered";
        
        initialized = true;
    }
    
    if(state <= UndefinedPrintEngineState || state >= MaxPrintEngineState)
    {
        LOGGER.HandleError(UnknownPrintEngineState, false, NULL, state);
        return "";                                                              
    }
    return stateNames[state];
}

/// Gets the name of a print engine state machine UI sub-state
const char* PrinterStatus::GetSubStateName(UISubState substate)
{
    static bool initialized = false;
    static const char* substateNames[MaxUISubState];
    if(!initialized)
    {
        // initialize the array of state names
        substateNames[NoUISubState] = "NoUISubState";
        substateNames[NoPrintData] = "NoPrintData";
        substateNames[Downloading] = "Downloading";
        substateNames[Downloaded] = "Downloaded";
        substateNames[DownloadFailed] = "DownloadFailed";
        substateNames[HavePrintData] = "HavePrintData";
        substateNames[PrintCanceled] = "PrintCanceled";
        substateNames[PrintCompleted] = "PrintCompleted";
        substateNames[ExitingDoorOpen] = "ExitingDoorOpen";
        initialized = true;
    }
    
    if(substate < NoUISubState || substate >= MaxUISubState)
    {
        LOGGER.HandleError(UnknownPrintEngineSubState, false, NULL, substate);
        return "";                                                              
    }
    return substateNames[substate];
}