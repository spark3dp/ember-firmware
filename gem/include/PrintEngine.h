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

#define PULSE_PERIOD_SEC    (1)    // period of status updates while printing
#define DEFAULT_EXPOSURE_TIME_SEC (10) // default exposure time per layer
#define DEFAULT_MOTOR_TIMEOUT_SEC (60) // default timeout for motor command completion

class PrinterStateMachine;

/// The class that controls the printing process
class PrintEngine : public ICallback
{
public: 
    PrintEngine();
    ~PrintEngine();
    void SendStatus(const char* stateName);
    void SetNumLayers(int numLayers);
    int NextLayer();
    bool NoMoreLayers();
    void EnablePulseTimer(bool enable);
    int GetPulseTimerFD();
    int GetExposureTimerFD();
    int GetMotorTimeoutTimerFD();
    void StartExposureTimer();
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    virtual void Callback(EventType eventType, void*);
    int GetStatusUpdateFD();
    void Initialize();
#ifdef DEBUG
    // for unit testing only
    PrinterStateMachine* GetStateMachine() { return _pPrinterStateMachine; }
#endif
    
private:
    PrinterStatus _status;
    int _pulseTimerFD;
    int _pulsePeriodSec;
    int _exposureTimerFD;    
    int _motorTimeoutTimerFD;
    int _statusReadFD;
    int _statusWriteFd;
    PrinterStatus _printerStatus;
    PrinterStateMachine* _pPrinterStateMachine;

    int GetExposureTimeSec();
}; 

#endif	/* PRINTENGINE_H */

