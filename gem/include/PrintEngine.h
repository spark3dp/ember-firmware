/* 
 * File:   PrintEngine.h
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

#include <PrinterStatus.h>
#include <Event.h>
#include <Motor.h>
#include <FrontPanel.h>

#define PULSE_PERIOD_SEC    (1)    // period of status updates while printing
#define DEFAULT_EXPOSURE_TIME_SEC (10) // default exposure time per layer
#define SEPARATION_TIME_SEC (3) // time required to separate from each layer
#define DEFAULT_MOTOR_TIMEOUT_SEC (30) // default timeout for motor command completion
#define LONGER_MOTOR_TIMEOUT_SEC (60) // timeout for longer motor command completion
#define LONGEST_MOTOR_TIMEOUT_SEC (120) // timeout for longest motor command completion

class PrinterStateMachine;

/// The class that controls the printing process
class PrintEngine : public ICallback
{
public: 
    PrintEngine(bool haveHardware);
    ~PrintEngine();
    void SendStatus(const char* stateName, StateChange change = NoChange);
    void SetNumLayers(int numLayers);
    int NextLayer();
    int CurrentLayer() { return _printerStatus._currentLayer; }
    bool NoMoreLayers();
    void SetEstimatedPrintTime(bool set);
    void UpdateRemainingPrintTime();
    void EnablePulseTimer(bool enable);
    int GetPulseTimerFD();
    int GetExposureTimerFD();
    int GetMotorTimeoutTimerFD();
    void StartExposureTimer(int seconds);
    void ClearExposureTimer();
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    virtual void Callback(EventType eventType, void*);
    int GetStatusUpdateFD();
    void Initialize();
    void SendMotorCommand(unsigned char command);
    void Begin();
    void CancelPrint();
    int GetExposureTimeSec();
    int GetRemainingExposureTimeSec();
    void StopMotor();
    bool DoorIsOpen();
#ifdef DEBUG
    // for testing only 
    PrinterStateMachine* GetStateMachine() { return _pPrinterStateMachine; }
#endif
    
private:
    int _pulseTimerFD;
    int _pulsePeriodSec;
    int _exposureTimerFD;    
    int _motorTimeoutTimerFD;
    int _statusReadFD;
    int _statusWriteFd;
    PrinterStatus _printerStatus;
    PrinterStateMachine* _pPrinterStateMachine;
    Motor* _pMotor;
    FrontPanel* _pFrontPanel;
    long _printStartedTimeMs;
    int _initialEstimatedPrintTime;

    PrintEngine(); // need to specify if we have hardware in c'tor

    void ButtonCallback();
    void MotorCallback();
    void DoorCallback(void* data);
    void KeyboardCallback(void* data);
    void HandleError(const char* errorMsg, bool fatal = false);
}; 

#endif	/* PRINTENGINE_H */

