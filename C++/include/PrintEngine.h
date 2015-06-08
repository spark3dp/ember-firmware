/* 
 * File:   PrintEngine.h
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

#include <map>

#include <PrinterStatus.h>
#include <Event.h>
#include <Motor.h>
#include <FrontPanel.h>
#include <Commands.h>
#include <Projector.h>
#include <ErrorMessage.h>
#include <Thermometer.h>
#include <LayerSettings.h>

// high-level motor commands, that may result in multiple low-level commands
#define HOME_COMMAND                            (1)
#define MOVE_TO_START_POSN_COMMAND              (2)
#define SEPARATE_COMMAND                        (3)
#define APPROACH_COMMAND                        (4)
#define APPROACH_AFTER_JAM_COMMAND              (5)
#define PAUSE_AND_INSPECT_COMMAND               (6)
#define RESUME_FROM_INSPECT_COMMAND             (7)
#define JAM_RECOVERY_COMMAND                    (8)

#define TEMPERATURE_MEASUREMENT_INTERVAL_SEC    (20.0)

class PrinterStateMachine;

/// The class that controls the printing process
class PrintEngine : public ICallback, public ICommandTarget
{
public: 
    PrintEngine(bool haveHardware);
    ~PrintEngine();
    void SendStatus(PrintEngineState state, StateChange change = NoChange, 
                    UISubState substate = NoUISubState);
    void SetNumLayers(int numLayers);
    bool NextLayer();
    int GetCurrentLayerNum() { return _printerStatus._currentLayer; }
    int GetNextLayerNum() { return GetCurrentLayerNum() + 1; }
    LayerType GetCurrentLayerType();
    int SetCurrentLayer(int layer) { _printerStatus._currentLayer = layer; }
    bool NoMoreLayers();
    void SetEstimatedPrintTime(bool set);
    void DecreaseEstimatedPrintTime(double amount);
    int GetPreExposureDelayTimerFD() { return _preExposureDelayTimerFD;}
    int GetExposureTimerFD() { return _exposureTimerFD;}
    int GetMotorTimeoutTimerFD() { return _motorTimeoutTimerFD; }
    int GetTemperatureTimerFD() { return _temperatureTimerFD; }
    void StartExposureTimer(double seconds);
    void ClearExposureTimer();
    void StartPreExposureDelayTimer(double seconds);
    void ClearPreExposureDelayTimer();
    void StartTemperatureTimer(double seconds);
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    int GetStatusUpdateFD() { return _statusReadFD; }
    void Initialize();
    void SendMotorCommand(int command);
    void Begin();
    void ClearCurrentPrint();
    double GetExposureTimeSec();
    double GetPreExposureDelayTimeSec();
    double GetSeparationTimeSec(LayerType type);
    double GetApproachTimeSec(LayerType type);
    int GetSeparationTimeoutSec();
    int GetApproachTimeoutSec();
    int GetPauseAndInspectTimeoutSec(bool toInspect);
    int GetHomingTimeoutSec();
    int GetStartPositionTimeoutSec();
    int GetUnjammingTimeoutSec();
    double GetRemainingExposureTimeSec();
    bool DoorIsOpen();
    I2C_Device* GetMotorController() { return _pMotor; }
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
    bool CanInspect();
    int GetInspectRotation();
    void SetInspectionRequested(bool requested);
    bool PauseRequested() {return _inspectionRequested; }
    double GetTemperature() { return _temperature; }
    bool SkipCalibration() { return _skipCalibration; }
    void SetSkipCalibration() { _skipCalibration = true; }
    void PauseMovement();
    void ResumeMovement();
    void ClearPendingMovement();
    int  PadTimeout(double rawTime);

#ifdef DEBUG
    // for testing only 
    PrinterStateMachine* GetStateMachine() { return _pPrinterStateMachine; }
#endif
    
private:
    int _preExposureDelayTimerFD;
    int _exposureTimerFD;    
    int _motorTimeoutTimerFD;
    int _temperatureTimerFD;
    int _statusReadFD;
    int _statusWriteFd;
    PrinterStatus _printerStatus;
    PrinterStateMachine* _pPrinterStateMachine;
    Motor* _pMotor;
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
    LayerSettings _layerSettings;
    int _currentZPosition;

    PrintEngine(); // need to specify if we have hardware in c'tor
    virtual void Callback(EventType eventType, void* data);
    virtual void Handle(Command command);
    void MotorCallback(unsigned char *status);
    void ButtonCallback(unsigned char* status);
    void DoorCallback(char* data);
    bool IsFirstLayer();
    bool IsBurnInLayer();
    void HandleDownloadFailed(ErrorCode errorCode, const char* jobName);
    void ProcessData();
    bool ShowHomeScreenFor(UISubState substate);
    void DeleteTempSettingsFile();
    double GetLayerTimeSec(LayerType type);
    bool IsPrinterTooHot();
    void LogStatusAndSettings();
}; 

#endif	/* PRINTENGINE_H */

