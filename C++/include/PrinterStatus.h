/* 
 * File:   PrinterStatus.h
 * Author: Richard Greene
 * 
 * The data structure used to communicate status from the print engine to UI 
 * components.
 *
 * Created on April 1, 2014, 3:09 PM
 */

#ifndef PRINTERSTATUS_H
#define	PRINTERSTATUS_H

#include <stddef.h>
#include <string>

#include <ErrorMessage.h>


#define STATE_NAME  PrinterStatus::GetStateName
#define SUBSTATE_NAME  PrinterStatus::GetSubStateName

/// the possible states in the print engine's state machine
enum PrintEngineState
{
    // undefined state, should never be used
    UndefinedPrintEngineState,
    
    PrinterOnState,
    DoorClosedState,
    InitializingState,
    DoorOpenState,
    HomingState,
    HomeState,
    ErrorState,
    PrintSetupState,
    MovingToStartPositionState,
    ExposingState,
    PrintingLayerState,
    RotatingForPauseState,
    MovingToPauseState,
    PausedState,
    RotatingForResumeState,
    MovingToResumeState,
    SeparatingState,
    ConfirmCancelState,
    ShowingVersionState,
    CalibrateState,
    MovingToCalibrationState,
    CalibratingState,
    EndingCalibrationState,
    RegisteringState,
    
    // Guardrail for valid states
    MaxPrintEngineState
};

/// the possible changes in state
enum StateChange
{
    NoChange,
    Entering,
    Leaving,
};

/// The possible sub-states that determine which of multiple screens are shown
/// for a single PrintEngineState
enum UISubState 
{
    NoUISubState,
    
    NoPrintData,
    DownloadingPrintData,
    PrintDownloadFailed,
    LoadingPrintData,
    LoadedPrintData,
    PrintDataLoadFailed,
    HavePrintData,
    PrintCanceled,
    PrintCompleted,
    ExitingDoorOpen,
    RotationJammed,
    Registered,
    AboutToPause,
    
    // Guardrail for valid sub-states
    MaxUISubState
};

class PrinterStatus
{
public: 
    PrinterStatus();
    static const char* GetStateName(PrintEngineState state);
    static const char* GetSubStateName(UISubState substate);
    std::string ToString();
    static void SetLastErrorMsg(std::string msg);
    static std::string GetLastErrorMessage();
    
    PrintEngineState _state;
    StateChange _change;
    UISubState _UISubState;
    bool _isError;
    ErrorCode _errorCode;
    int _errno;
    int _numLayers;
    int _currentLayer;
    int _estimatedSecondsRemaining;
    double _temperature;
};

#endif	/* PRINTERSTATUS_H */

