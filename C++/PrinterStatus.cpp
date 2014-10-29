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
_temperature(0.0)
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
        stateNames[PrinterOnState] = PRINTER_ON_STATE;
        stateNames[DoorClosedState] = DOOR_CLOSED_STATE;
        stateNames[InitializingState] = INITIALIZING_STATE;
        stateNames[DoorOpenState] = DOOR_OPEN_STATE;
        stateNames[HomingState] = HOMING_STATE;
        stateNames[HomeState] = HOME_STATE;
        stateNames[ErrorState] = ERROR_STATE;
        stateNames[PrintSetupState] = PRINT_SETUP_STATE;
        stateNames[MovingToStartPositionState] = MOVING_TO_START_POSITION_STATE;
        stateNames[ExposingState] = EXPOSING_STATE;
        stateNames[PrintingLayerState] = PRINTING_LAYER_STATE;
        stateNames[PausedState] = PAUSED_STATE;
        stateNames[SeparatingState] = SEPARATING_STATE;
        stateNames[EndingPrintState] = ENDING_PRINT_STATE;  
        stateNames[ConfirmCancelState] = CONFIRM_CANCEL_STATE;
        stateNames[ShowingVersionState] = SHOWING_VERSION_STATE;
        stateNames[CalibrateState] = CALIBRATE_STATE;
        stateNames[MovingToCalibrationState] = MOVING_TO_CALIBRATION_STATE;
        stateNames[CalibratingState] = CALIBRATING_STATE;
        stateNames[RegisteringState] = REGISTERING_STATE;
        stateNames[RegisteredState] = REGISTERED_STATE;
        
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
        substateNames[NoUISubState] = NO_SUBSTATE;
        substateNames[NoPrintData] = NO_PRINT_DATA_SUBSTATE;
        substateNames[Downloading] = DOWNLOADING_SUBSTATE;
        substateNames[Downloaded] = DOWNLOADED_SUBSTATE;
        substateNames[DownloadFailed] = DOWNLOAD_FAILED_SUBSTATE;
        substateNames[HavePrintData] = HAVE_PRINT_DATA_SUBSTATE;
        substateNames[PrintCanceled] = PRINT_CANCELED_SUBSTATE;
        substateNames[PrintCompleted] = PRINT_COMPLETED_SUBSTATE;
        substateNames[ExitingDoorOpen] = EXITING_DOOR_OPEN_SUBSTATE;
        substateNames[RotationJammed] = ROTATION_JAMMED_SUBSTATE;
        initialized = true;
    }
    
    if(substate < NoUISubState || substate >= MaxUISubState)
    {
        LOGGER.HandleError(UnknownPrintEngineSubState, false, NULL, substate);
        return "";                                                              
    }
    return substateNames[substate];
}