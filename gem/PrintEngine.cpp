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
#include <fcntl.h>
#include <sys/stat.h>

#include <Hardware.h>
#include <MessageStrings.h>
#include <PrintEngine.h>
#include <PrinterStateMachine.h>

/// Constructor
PrintEngine::PrintEngine() :
_pulseTimerFD(-1),
_pulsePeriodSec(PULSE_PERIOD_SEC),
_exposureTimerFD(-1),
_motorTimeoutTimerFD(-1),
_statusReadFD(-1),
_statusWriteFd(-1)        
{
    // the print engine "owns" its timers,
    //so it can enable and disable them as needed
    _pulseTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_pulseTimerFD < 0)
    {
        perror(PULSE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _exposureTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_exposureTimerFD < 0)
    {
        perror(EXPOSURE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _motorTimeoutTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_motorTimeoutTimerFD < 0)
    {
        perror(MOTOR_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    // the print engine also "owns" the status update FIFO
    char pipeName[] = "/tmp/PrinterStatusPipe";
    // don't recreate the FIFO if it exists already
    if (access(pipeName, F_OK) == -1) {
        if (mkfifo(pipeName, 0666) < 0) {
          perror(STATUS_PIPE_CREATION_ERROR);
          return;
        }
    }
    // Open both ends within this process in on-blocking mode,
    // otherwise open call would wait till other end of pipe
    // is opened by another process
    _statusReadFD = open(pipeName, O_RDONLY|O_NONBLOCK);
    _statusWriteFd = open(pipeName, O_WRONLY|O_NONBLOCK);
    
    // construct the state machine and tell it this print engine owns it
    _pPrinterStateMachine = new PrinterStateMachine(this);
    // start the state machine
    _pPrinterStateMachine->initiate();
  
    // create the I2C devices for the motor & UI boards
    // use 0xFF as slave address for testing without actual boards
    _pMotor = new Motor(MOTOR_SLAVE_ADDRESS); 
    _pFrontPanel = new FrontPanel(UI_SLAVE_ADDRESS);
}

PrintEngine::~PrintEngine()
{
    // the state machine gets deleted without the following call, which
    // therefore causes an error
 //   delete _pPrinterStateMachine;
    
    delete _pMotor;
    delete _pFrontPanel;
}

/// Perform initialization that will be repeated whenever the state machine 
/// enters the Initializing state
void PrintEngine::Initialize()
{
    // TODO: realistic initialization of printer status
#ifdef DEBUG
    _printerStatus._currentLayer = 0;
    _printerStatus._numLayers = 1000;
    _printerStatus._estimatedSecondsRemaining = 600;
#endif   
    
    ClearMotorTimeoutTimer();
}

/// Translate the event handler events into state machine events
void PrintEngine::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case ButtonInterrupt:
           ButtonCallback();
           break;

        case MotorInterrupt:
            MotorCallback();
            break;

        case DoorInterrupt:
            DoorCallback(data);
            break;

        case PrintEnginePulse:
            _pPrinterStateMachine->process_event(EvPulse());
            break;
            
        case ExposureEnd:
            _pPrinterStateMachine->process_event(EvExposed());
            break;
            
        case MotorTimeout:
            // TODO: provide more details about which motor action timed out
            HandleError(MOTOR_TIMEOUT_ERROR, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}
 
/// Gets the file descriptor used for the status pulse timer
int PrintEngine::GetPulseTimerFD()
{
    return _pulseTimerFD;
}
   
/// Gets the file descriptor used for the exposure timer
int PrintEngine::GetExposureTimerFD()
{
    return _exposureTimerFD;
}

/// Gets the file descriptor used for the motor board timeout timer
int PrintEngine::GetMotorTimeoutTimerFD()
{
    return _motorTimeoutTimerFD;
}
   
/// Gets the file descriptor used for the status update named pipe
int PrintEngine::GetStatusUpdateFD()
{
    return _statusReadFD;
}

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

/// Start the timer whose expiration signals the end of exposure for a layer
void PrintEngine::StartExposureTimer()
{
    struct itimerspec timer1Value;
    
    timer1Value.it_value.tv_sec = GetExposureTimeSec();
    timer1Value.it_value.tv_nsec = 0;
    timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    timer1Value.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_exposureTimerFD, 0, &timer1Value, NULL) == -1)
    {
        perror(EXPOSURE_TIMER_ERROR);  
        exit(-1);
    }
}

/// Get the exposure time for the current layer
int PrintEngine::GetExposureTimeSec()
{
    // TODO: determine the desired exposure time for the current layer
    // for now just
    return DEFAULT_EXPOSURE_TIME_SEC;
}

/// Start the timer whose expiration signals that the motor board has not 
// indicated that its completed a command in the expected time
void PrintEngine::StartMotorTimeoutTimer(int seconds)
{
    struct itimerspec timer1Value;
    
    timer1Value.it_value.tv_sec = seconds;
    timer1Value.it_value.tv_nsec = 0;
    timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    timer1Value.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_motorTimeoutTimerFD, 0, &timer1Value, NULL) == -1)
    {
        perror(EXPOSURE_TIMER_ERROR);  
        exit(-1);
    }
}

/// Clears the timer whose expiration signals that the motor board has not 
// indicated that its completed a command in the expected time
void PrintEngine::ClearMotorTimeoutTimer()
{
    // setting a 0 as the time disarms the timer
    StartMotorTimeoutTimer(0);
}

/// Send out the status of the print engine, 
/// including status of any print in progress
void PrintEngine::SendStatus(const char* stateName)
{
    _status._state = stateName;
#ifdef DEBUG
    // in debug build, print out what state we're in
    std::cout << _status._state << std::endl; 
#endif

    if(_statusWriteFd >= 0)
    {
#ifdef DEBUG  
        // in debug build, show some changes
        _printerStatus._currentLayer++;
        _printerStatus._estimatedSecondsRemaining--;
#endif
        // send status info out the PE status pipe
        lseek(_statusWriteFd, 0, SEEK_SET);
        write(_statusWriteFd, &_printerStatus, sizeof(struct PrinterStatus)); 
    }
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

/// Translates button events from UI board into state machine events
void PrintEngine::ButtonCallback()
{
    // read the UI board's status register
    unsigned char status = _pFrontPanel->Read(UI_STATUS);
    
    // forward the translated event, or pass it on to the state machine when
    // the translation requires knowledge of the current state
    switch(status)
    {
        case ERROR_STATUS:
            HandleError(FRONT_PANEL_ERROR);
            break;
            
        case BTN1_PRESS:
            // either start a print or cancel one in progress
            _pPrinterStateMachine->StartOrCancelPrint();
            break;
            
        case BTN1_HOLD:
            // reset
            _pPrinterStateMachine->process_event(EvReset());
            break;
            
        case BTN2_PRESS:          
            // either pause or resume
            _pPrinterStateMachine->PauseOrResume();
            break;
            
        case BTN2_HOLD:
            // either sleep or wake
            _pPrinterStateMachine->SleepOrWake();
            break;
            
        case BTN3_PRESS:      
        case BTN3_HOLD:
            break;  // button 3 not currently used
            
        default:
            perror(FormatError("Unknown front panel status: %d", status));
            break;
    }
}

/// Translates interrupts from motor board into state machine events
void PrintEngine::MotorCallback()
{
    // read the motor board's status register
    unsigned char status = _pMotor->Read(MOTOR_STATUS);
    
    // forward the translated event, or pass it on to the state machine when
    // the translation requires knowledge of the current state
    switch(status)
    {
        case ERROR_STATUS:
            HandleError(MOTOR_ERROR, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
            
        case SUCCESS:
            _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            perror(FormatError(UNKNOWN_MOTOR_STATUS, status));
            break;
    }    
}

/// Translates door button interrupts into state machine events
void PrintEngine::DoorCallback(void* data)
{
    char received = *((char*) data);
    // TODO: make sure we have the polarity on this right!
    if(received == '1')
        _pPrinterStateMachine->process_event(EvDoorOpened());
    else
        _pPrinterStateMachine->process_event(EvDoorClosed());
}
 
/// Handles errors
void PrintEngine::HandleError(const char* errorMsg, bool fatal)
{
    // TODO: we probably want to accept an error code instead of or in addition 
    // to a simple message, possibly with other relevant data as well.
    // We'll also want to make sure this error is logged.
    // For now, just print the error, and set the state machine into 
    // the Idle state.
    perror(errorMsg);
    
    if(fatal)
        _pPrinterStateMachine->process_event(EvError());
}
 
/// Send a single-character command to the motor board
void PrintEngine::SendMotorCommand(unsigned char command)
{
    _pMotor->Write(MOTOR_COMMAND, command);
}



