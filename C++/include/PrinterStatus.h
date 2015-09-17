//  File:   PrinterStatus.h
//  Defines the data structure used to communicate status from the print engine 
//  to UI components
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

#ifndef PRINTERSTATUS_H
#define	PRINTERSTATUS_H

#include <stddef.h>
#include <string>

#include <ErrorMessage.h>
#include <utils.h>


#define STATE_NAME  PrinterStatus::GetStateName
#define SUBSTATE_NAME  PrinterStatus::GetSubStateName
#define PS_KEY  PrinterStatus::GetKey

typedef int PrinterStatusKey;

// the possible states in the print engine's state machine
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
    MovingToStartPositionState,
    PressingState,
    PressDelayState,
    UnpressingState,
    PreExposureDelayState,
    ExposingState,
    PrintingLayerState,
    MovingToPauseState,
    PausedState,
    MovingToResumeState,
    SeparatingState,
    ApproachingState,
    GettingFeedbackState,
    ConfirmCancelState,
    AwaitingCancelationState,
    ShowingVersionState,
    CalibratingState,
    RegisteringState,
    UnjammingState,
    JammedState,
    DemoModeState,
    
    // Guardrail for valid states
    MaxPrintEngineState
};

// the possible changes in state
enum StateChange
{
    NoChange,
    Entering,
    Leaving,
};

// The possible sub-states that determine which of multiple screens are shown
// for a single PrintEngineState
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
    Registered,
    AboutToPause,
    WiFiConnecting,
    WiFiConnectionFailed,
    WiFiConnected,
    CalibratePrompt,
    USBDriveFileFound,
    USBDriveError,

    // Guardrail for valid sub-states
    MaxUISubState
};

// the possible print feedback values a user may supply
enum PrintRating
{
    Unknown = 0,

    Succeeded,
    Failed,
};

class PrinterStatus
{
public: 
    PrinterStatus();
    static const char* GetStateName(PrintEngineState state);
    static const char* GetSubStateName(UISubState substate);
    std::string ToString() const;
    static void SetLastErrorMsg(std::string msg);
    static std::string GetLastErrorMessage();
    static PrinterStatusKey GetKey(PrintEngineState state, UISubState subState);

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
    char _localJobUniqueID[UUID_LEN + 1];
    PrintRating _printRating;
    std::string _usbDriveFileName;
    std::string _jobID;
};

#endif    // PRINTERSTATUS_H

