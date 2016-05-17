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
#include <Magick++.h>

#include <PrinterStatus.h>
#include <Motor.h>
#include <FrontPanel.h>
#include <Command.h>
#include <ErrorMessage.h>
#include <Thermometer.h>
#include <LayerSettings.h>
#include <ImageProcessor.h>
#include <Settings.h>

// high-level motor commands, that may result in multiple low-level commands
enum HighLevelMotorCommand
{
    GoHome,
    GoHomeWithoutRotateHome,
    MoveToStartPosition,
    Press,                 
    UnPress,               
    Separate,              
    Approach,
    ApproachAfterJam,
    PauseAndInspect,
    ResumeFromInspect,
    RecoverFromJam
};

constexpr double TEMPERATURE_MEASUREMENT_INTERVAL_SEC = 20.0;

class PrinterStateMachine;
class PrintData;
class Projector;
class PrinterStatusQueue;
class Timer;
class Projector;

// Aggregates the data used by the background thread.
struct ThreadData 
{
    Magick::Image* pImage;
    PrintData*  pPrintData;
    int         layer;
    ImageProcessor* imageProcessor;
    Projector*  pProjector;
    double      scaleFactor;
    bool        usePatternMode;
};


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
    PrintEngine(bool haveHardware, Motor& motor, Projector& projector,
            PrinterStatusQueue& printerStatusPipe,
            const Timer& exposureTimer, const Timer& temperatureTimer,
            const Timer& delayTimer, const Timer& motorTimeoutTimer);
    ~PrintEngine();
    void SendStatus(PrintEngineState state, StateChange change = NoChange, 
                    UISubState substate = NoUISubState);
    void SetNumLayers(int numLayers);
    void NextLayer();
    int GetCurrentLayerNum() { return _printerStatus._currentLayer; }
    int SetCurrentLayer(int layer) { _printerStatus._currentLayer = layer; }
    bool MoreLayers();
    void SetEstimatedPrintTime();
    void StartExposureTimer(double seconds);
    void ClearExposureTimer();
    void StartDelayTimer(double seconds);
    void ClearDelayTimer();
    void StartTemperatureTimer(double seconds);
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    void Initialize();
    void SendMotorCommand(HighLevelMotorCommand command);
    void Begin();
    void ClearCurrentPrint(bool withInterrupt = false);
    double GetExposureTimeSec();
    double GetPreExposureDelayTimeSec();
    bool NeedsPreExposureDelay();
    double GetRemainingExposureTimeSec();
    bool DoorIsOpen();
    void ShowImage();
    void TurnProjectorOff();
    bool TryStartPrint();
    bool SendSettings();
    bool HandleError(ErrorCode code, bool fatal = false, 
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
    bool NeedsTrayDeflectionPause();
    void GetCurrentLayerSettings();
    void DisableMotors() { _motor.DisableMotors(); }
    void SetPrintFeedback(PrintRating rating);
    bool PrintIsInProgress() { return _printerStatus._numLayers != 0; }
    bool DemoModeRequested();
    bool SetDemoMode();
    void LoadPrintFileFromUSBDrive();
    bool LoadNextLayerImage();
    bool AwaitEndOfBackgroundThread(bool ignoreErrors = false);
    void SetCanLoadPrintData(bool canLoad);
    bool ShowScreenFor(UISubState substate);
    bool CanUpgradeProjector() { return _printerStatus._canUpgradeProjector; }
    bool PutProjectorInProgramMode(bool enter);
    void UpgradeProjectorFirmware();
    bool ProjectorProgrammingCompleted();
    bool SetPrintMode();

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
    ImageProcessor _imageProcessor;
    pthread_t _bgndThread;
    ThreadData _threadData;
    Magick::Image _image;
    static ErrorCode _threadError;
    static const char* _threadErrorMsg;

    PrinterStatusQueue& _printerStatusQueue;
    const Timer& _exposureTimer;
    const Timer& _temperatureTimer;
    const Timer& _delayTimer;
    const Timer& _motorTimeoutTimer;
    Projector& _projector;
    Settings& _settings;

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
    static void* InBackground(void* context);
}; 

#endif    // PRINTENGINE_H

