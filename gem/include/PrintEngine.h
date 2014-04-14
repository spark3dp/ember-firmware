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

/// We will always need one and only one PrintEngine, 
/// so it is defined as a singleton
class PrintEngine : public ICallback
{
public: 
    static PrintEngine& Instance()
    { 
        static PrintEngine _theOneAndOnly;
        return _theOneAndOnly;
    }

    void SendStatus(const char* stateName);
    void SetNumLayers(int numLayers);
    int NextLayer();
    bool NoMoreLayers();
    void EnablePulseTimer(bool enable);
    int GetPulseTimerFD();
    void StartExposureTimer();
    void StartMotorTimeoutTimer(int seconds);
    void ClearMotorTimeoutTimer();
    virtual void Callback(EventType eventType, void*);
    int GetStatusUpdateFD();
    void Initialize();
    
private:
    PrinterStatus _status;
    int _pulseTimerFD;
    int _pulsePeriodSec;
    int _exposureTimerFD;    
    int _motorTimeoutTimerFD;
    int _statusReadFD;
    int _statusWriteFd;
    PrinterStatus _printerStatus;

    int GetExposureTimeSec();
    
    // Disallow construction, copying, or assignment 
    PrintEngine();
    PrintEngine(PrintEngine const&);
    PrintEngine& operator=(PrintEngine const&);
}; 

#endif	/* PRINTENGINE_H */

