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

#define STATE_NAME  PrinterStatus::StateName

// keys for PrinterStatus
#define PRINTER_STATUS_KEY      "PrinterStatus"
#define STATE_PS_KEY            "State"
#define CHANGE_PS_KEY           "Change"
#define IS_ERROR_PS_KEY         "IsError"
#define IS_FATAL_ERROR_PS_KEY   "IsFatalError"
#define ERROR_CODE_PS_KEY       "ErrorCode"
#define ERRNO_PS_KEY            "Errno"
#define ERROR_PS_KEY            "Error"
#define LAYER_PS_KEY            "Layer"
#define TOAL_LAYERS_PS_KEY      "TotalLayers"
#define SECONDS_LEFT_PS_KEY     "SecondsLeft"
#define JOB_NAME_PS_KEY         "JobName"
#define TEMPERATURE_PS_KEY      "Temperature"
#define UISUBSTATE_PS_KEY       "UISubState"

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
    IdleState,
    PrintSetupState,
    MovingToStartPositionState,
    ExposingState,
    PrintingState,
    PausedState,
    SeparatingState,
    EndingPrintState,
    ConfirmCancelState,
    
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
    
    Downloading,
    Downloaded,
    DownloadFailed,
    
    MaxUISubState
};

class PrinterStatus
{
public: 
    PrinterStatus();
    static const char* StateName(PrintEngineState state);
    
    PrintEngineState _state;
    StateChange _change;
    UISubState _UISubState;
    bool _isError;
    bool _isFatalError;
    int _errorCode;
    int _errno;
    const char* _errorMessage;
    int _numLayers;
    int _currentLayer;
    int _estimatedSecondsRemaining;
    const char* _jobName;
    float _temperature;
};

#endif	/* PRINTERSTATUS_H */

