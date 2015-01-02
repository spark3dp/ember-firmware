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

// timeouts for motor command completion
#define DEFAULT_MOTOR_TIMEOUT_SEC (30) 
#define LONGER_MOTOR_TIMEOUT_SEC (60) 
#define LONGEST_MOTOR_TIMEOUT_SEC (120) 
#define BASE_SEPARATION_MOTOR_TIMEOUT_SEC (15) 
#define TEMPERATURE_MEASUREMENT_INTERVAL_SEC (20.0)

/// The different types of layers that may be printed
enum LayerType
{
    First,
    BurnIn,
    Model
};

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
    int GetCurrentLayer() { return _printerStatus._currentLayer; }
    int SetCurrentLayer(int layer) { _printerStatus._currentLayer = layer; }
    bool NoMoreLayers();
    void SetEstimatedPrintTime(bool set);
    void DecreaseEstimatedPrintTime(double amount);
    int GetExposureTimerFD() { return _exposureTimerFD;}
    int GetMotorTimeoutTimerFD() { return _motorTimeoutTimerFD; }
    int GetTemperatureTimerFD() { return _temperatureTimerFD; }
    void StartExposureTimer(double seconds);
    void ClearExposureTimer();
    void StartTemperatureTimer(double seconds);
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    int GetStatusUpdateFD() { return _statusReadFD; }
    void Initialize();
    void SendMotorCommand(unsigned char command);
    void SendMotorCommand(const char* commandFormatString, int value);
    void Begin();
    void ClearCurrentPrint();
    double GetExposureTimeSec();
    char GetSeparationCommand();
    int GetSeparationTimeoutSec();
    double GetRemainingExposureTimeSec();
    bool DoorIsOpen();
    I2C_Device* GetMotorBoard() { return _pMotor; }
    void ShowImage();
    void ShowBlack();
    bool TryStartPrint();
    bool SendSettings();
    void HandleError(ErrorCode code, bool fatal = false, 
                     const char* str = NULL, int value = INT_MAX);
    void ClearError();
    void PowerProjector(bool on);
    bool HasAtLeastOneLayer();
    UISubState GetUISubState();
    void ClearPrintData();
    UISubState GetHomeUISubState() { return _homeUISubState; } 
    void ClearHomeUISubState() { _homeUISubState = NoUISubState; } 
    bool CancelRequested() { return _cancelRequested; }
    void ClearRotationInterrupt() { _gotRotationInterrupt = false; }
    bool GotRotationInterrupt(); 
    void ClearJobID();
    bool CanInspect();
    int GetPauseRotation();
    void SetPauseRequested(bool requested);
    bool PauseRequested() {return _pauseRequested; }

#ifdef DEBUG
    // for testing only 
    PrinterStateMachine* GetStateMachine() { return _pPrinterStateMachine; }
#endif
    
private:
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
    Projector _projector;
    bool _awaitingMotorSettingAck;
    std::map<const char*, const char*> _motorSettings;
    bool _haveHardware;
    UISubState _homeUISubState;
    bool _invertDoorSwitch;
    double _temperature;
    Thermometer* _pThermometer;
    bool _cancelRequested;
    bool _gotRotationInterrupt;
    bool _alreadyOverheated;
    bool _pauseRequested;

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
    double GetLayerTime(LayerType type);
    bool IsPrinterTooHot();
    void LogStatusAndSettings();
}; 

#endif	/* PRINTENGINE_H */

