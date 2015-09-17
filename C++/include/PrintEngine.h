//  File:   PrintEngine.h
//  Defines the engine that controls the printing process
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

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

#include <map>
#include <boost/scoped_ptr.hpp>

#include <PrinterStatus.h>
#include <Motor.h>
#include <FrontPanel.h>
#include <Command.h>
#include <Projector.h>
#include <ErrorMessage.h>
#include <Thermometer.h>
#include <LayerSettings.h>

// high-level motor commands, that may result in multiple low-level commands
#define HOME_COMMAND                            (1)
#define MOVE_TO_START_POSN_COMMAND              (2)
#define PRESS_COMMAND                           (3)
#define UNPRESS_COMMAND                         (4)
#define SEPARATE_COMMAND                        (5)
#define APPROACH_COMMAND                        (6)
#define APPROACH_AFTER_JAM_COMMAND              (7)
#define PAUSE_AND_INSPECT_COMMAND               (8)
#define RESUME_FROM_INSPECT_COMMAND             (9)
#define JAM_RECOVERY_COMMAND                    (10)

#define TEMPERATURE_MEASUREMENT_INTERVAL_SEC    (20.0)

class PrinterStateMachine;
class PrintData;
class PrinterStatusQueue;
class Timer;

// The different types of layers that may be printed
enum LayerType
{
    First,
    BurnIn,
    Model
};

// The class that controls the printing process
class PrintEngine : public ICallback, public ICommandTarget
{
public: 
    PrintEngine(bool haveHardware, Motor& motor, 
            PrinterStatusQueue& printerStatusPipe,
            const Timer& exposureTimer, const Timer& temperatureTimer,
            const Timer& delayTimer, const Timer& motorTimeoutTimer);
    ~PrintEngine();
    void SendStatus(PrintEngineState state, StateChange change = NoChange, 
                    UISubState substate = NoUISubState);
    void SetNumLayers(int numLayers);
    bool NextLayer();
    int GetCurrentLayerNum() { return _printerStatus._currentLayer; }
    int SetCurrentLayer(int layer) { _printerStatus._currentLayer = layer; }
    bool NoMoreLayers();
    void SetEstimatedPrintTime(bool set);
    void DecreaseEstimatedPrintTime(double amount);
    void StartExposureTimer(double seconds);
    void ClearExposureTimer();
    void StartDelayTimer(double seconds);
    void ClearDelayTimer();
    void StartTemperatureTimer(double seconds);
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    void Initialize();
    void SendMotorCommand(int command);
    void Begin();
    void ClearCurrentPrint(bool withInterrupt = false);
    double GetExposureTimeSec();
    double GetPreExposureDelayTimeSec();
    double GetRemainingExposureTimeSec();
    bool DoorIsOpen();
    void ShowImage();
    void ShowBlack();
    bool TryStartPrint();
    bool SendSettings();
    void HandleError(ErrorCode code, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX);
    void ClearError();
    bool HasAtLeastOneLayer();
    UISubState GetUISubState();
    void ClearPrintData();
    UISubState GetHomeUISubState() { return _homeUISubState; } 
    void ClearHomeUISubState() { _homeUISubState = NoUISubState; } 
    void ClearRotationInterrupt() { _gotRotationInterrupt = false; }
    bool GotRotationInterrupt(); 
    void ClearJobID();
    void SetInspectionRequested(bool requested);
    bool PauseRequested() {return _inspectionRequested; }
    double GetTemperature() { return _temperature; }
    bool SkipCalibration() { return _skipCalibration; }
    void SetSkipCalibration() { _skipCalibration = true; }
    void PauseMovement();
    void ResumeMovement();
    void ClearPendingMovement(bool withInterrupt = false);
    int  PadTimeout(double rawTime);
    int GetTrayDeflection();
    double GetTrayDeflectionPauseTimeSec();
    void GetCurrentLayerSettings();
    void DisableMotors() { _motor.DisableMotors(); }
    void SetPrintFeedback(PrintRating rating);
    bool PrintIsInProgress() { return _printerStatus._numLayers != 0; }
    bool DemoModeRequested();
    bool SetDemoMode();
    void LoadPrintFileFromUSBDrive();

#ifdef DEBUG
    // for testing only 
    PrinterStateMachine* GetStateMachine() { return _pPrinterStateMachine; }
#endif
    
private:
    PrinterStatus _printerStatus;
    PrinterStateMachine* _pPrinterStateMachine;
    Motor& _motor;
    long _printStartedTimeMs;
    int _initialEstimatedPrintTime;
    Projector* _pProjector;
    std::map<const char*, const char*> _motorSettings;
    bool _haveHardware;
    UISubState _homeUISubState;
    bool _invertDoorSwitch;
    double _temperature;
    Thermometer* _pThermometer;
    bool _gotRotationInterrupt;
    bool _alreadyOverheated;
    bool _inspectionRequested;
    bool _skipCalibration;
    double _remainingMotorTimeoutSec;
    LayerSettings _perLayer;
    int _currentZPosition;
    CurrentLayerSettings _cls;
    boost::scoped_ptr<PrintData> _pPrintData;
    bool _demoModeRequested;

    PrinterStatusQueue& _printerStatusQueue;
    const Timer& _exposureTimer;
    const Timer& _temperatureTimer;
    const Timer& _delayTimer;
    const Timer& _motorTimeoutTimer;

    // This class has reference and pointer members
    // Disable copy construction and copy assignment
    PrintEngine(const PrintEngine&);
    PrintEngine& operator=(const PrintEngine&);

    virtual void Callback(EventType eventType, const EventData& data);
    virtual void Handle(Command command);
    void MotorCallback(unsigned char status);
    void ButtonCallback(unsigned char status);
    void DoorCallback(char data);
    bool IsFirstLayer();
    bool IsBurnInLayer();
    void HandleProcessDataFailed(ErrorCode errorCode, 
                                 const std::string& jobName);
    void ProcessData();
    bool ShowHomeScreenFor(UISubState substate);
    double GetLayerTimeSec(LayerType type);
    bool IsPrinterTooHot();
    void LogStatusAndSettings();
    int GetHomingTimeoutSec();
    int GetStartPositionTimeoutSec();
    int GetPauseAndInspectTimeoutSec(bool toInspect);
    int GetUnjammingTimeoutSec();
    int GetPressTimeoutSec();
    int GetUnpressTimeoutSec();
    int GetSeparationTimeoutSec();
    int GetApproachTimeoutSec();
    void USBDriveConnectedCallback(const std::string& deviceNode);
    void USBDriveDisconnectedCallback();

}; 

#endif    // PRINTENGINE_H

