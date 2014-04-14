/* 
 * File:   PrintEngine.cpp
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#include <stdio.h>
#include <iostream>
#include <sys/timerfd.h>
#include <stdlib.h>

#include <Hardware.h>
#include <MessageStrings.h>
#include <PrintEngine.h>

/// private constructor
PrintEngine::PrintEngine() :
_pulseTimerFD(-1),
_pulsePeriodSec(PULSE_PERIOD_SEC)        
{
    // the print engine "owns" the pulse timer,
    //so it can enable and disable it as needed
    _pulseTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_pulseTimerFD < 0)
    {
        perror(PULSE_TIMER_CREATE_ERROR);
        exit(-1);
    }
}

void PrintEngine::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case ButtonInterrupt:
           //_buttonCallback(data);
           break;

        case MotorInterrupt:
            //_motorCallback(data);
            break;

        case DoorInterrupt:
            //_doorCallback(data);
            break;

        case PrintEnginePulse:
            SendStatus("pulse");
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
    
int PrintEngine::GetPulseTimerFD()
{
    return _pulseTimerFD;
}
    
//int GetStatusUpdateFD()
//{
//    return _statusReadFD;
//}

/// Enable or disable the pulse timer used to signal when to send status updates while printing
void PrintEngine::EnablePulseTimer(bool enable)
{
    // set to automatically repeat or not depending on
    struct itimerspec timer1Value;
    timer1Value.it_value.tv_nsec = 0;
    timer1Value.it_interval.tv_nsec =0;
    
    if(enable)
    {
        timer1Value.it_value.tv_sec = _pulsePeriodSec;
        timer1Value.it_interval.tv_sec =_pulsePeriodSec; // automatically repeat
    }
    else
    {
        timer1Value.it_value.tv_sec = 0;
        timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    }
       
    // set relative timer
    if (timerfd_settime(_pulseTimerFD, 0, &timer1Value, NULL) == -1)
    {
        if(enable)
            perror(ENABLE_PULSE_TIMER_ERROR);
        else
            perror(DISABLE_PULSE_TIMER_ERROR);
    }
}

/// Send out the status of the print engine, 
/// including status of any print in progress
void PrintEngine::SendStatus(const char* stateName)
{
    _status._state = stateName;
    // TODO arrange to update and send actual status, using named pipe
    // for now, just print out what state we're in
    std::cout << _status._state << std::endl; 
}

/// Set the number of layers in the current print.  
/// Also clears the current layer number.
void PrintEngine::SetNumLayers(int numLayers)
{
    _status._numLayers = numLayers;
    // this assumes the number of layers is only set before starting a print
    _status._currentLayer = 0;
}

/// Increment the current layer number and return its value.
int PrintEngine::NextLayer()
{
    return(++_status._currentLayer);   
}

/// Returns true or false depending on whether or not the current print
/// has any more layers to be printed.
bool PrintEngine::NoMoreLayers()
{
    return _status._currentLayer > _status._numLayers;
}





