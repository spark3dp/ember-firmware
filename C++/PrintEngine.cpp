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
#include <Logger.h>
#include <Filenames.h>
#include <PrintData.h>

/// The only public constructor.  'haveHardware' can only be false in debug
/// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware) :
_pulseTimerFD(-1),
_pulsePeriodSec(PULSE_PERIOD_SEC),
_exposureTimerFD(-1),
_motorTimeoutTimerFD(-1),
_statusReadFD(-1),
_statusWriteFd(-1)    
{
#ifndef DEBUG
    if(!haveHardware)
    {
        Logger::LogError(LOG_ERR, errno, HARDWARE_NEEDED_ERROR);
        exit(-1);
    }
#endif  
    
    // the print engine "owns" its timers,
    //so it can enable and disable them as needed
    _pulseTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_pulseTimerFD < 0)
    {
        Logger::LogError(LOG_ERR, errno, PULSE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _exposureTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_exposureTimerFD < 0)
    {
        Logger::LogError(LOG_ERR, errno, EXPOSURE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _motorTimeoutTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_motorTimeoutTimerFD < 0)
    {
        Logger::LogError(LOG_ERR, errno, MOTOR_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    // the print engine also "owns" the status update FIFO
    // don't recreate the FIFO if it exists already
    if (access(PRINTER_STATUS_PIPE, F_OK) == -1) {
        if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0) {
          Logger::LogError(LOG_ERR, errno, STATUS_PIPE_CREATION_ERROR);
          exit(-1);  // we can't really run if we can't update clients on status
        }
    }
    // Open both ends within this process in non-blocking mode,
    // otherwise open call would wait till other end of pipe
    // is opened by another process
    _statusReadFD = open(PRINTER_STATUS_PIPE, O_RDONLY|O_NONBLOCK);
    _statusWriteFd = open(PRINTER_STATUS_PIPE, O_WRONLY|O_NONBLOCK);
    
    // create the I2C device for the motor board
    // use 0xFF as slave address for testing without actual boards
    // note, this must be defined before starting the state machine!
    _pMotor = new Motor(haveHardware ? MOTOR_SLAVE_ADDRESS : 0xFF); 
    
    // construct the state machine and tell it this print engine owns it
    _pPrinterStateMachine = new PrinterStateMachine(this);      
}

/// Destructor
PrintEngine::~PrintEngine()
{
    // the state machine apparently gets deleted without the following call, 
    // which therefore would cause an error
 //   delete _pPrinterStateMachine;
    
    delete _pMotor;
    
    if (access(PRINTER_STATUS_PIPE, F_OK) != -1)
        remove(PRINTER_STATUS_PIPE);    
}

/// Starts the printer state machine.  Should not be called until event handler
/// subscriptions are in place.
void PrintEngine::Begin()
{
    _pPrinterStateMachine->initiate();  
}

/// Perform initialization that will be repeated whenever the state machine 
/// enters the Initializing state
void PrintEngine::Initialize()
{
    // TODO: more complete initialization of printer status
    _printerStatus._state = "undefined";
    _printerStatus._currentLayer = 0;
    _printerStatus._estimatedSecondsRemaining = 0;
    
    ClearMotorTimeoutTimer();
}

/// Send out the status of the print engine, 
/// including status of any print in progress
void PrintEngine::SendStatus(const char* stateName, StateChange change)
{
    _printerStatus._state = stateName;
    _printerStatus._change = change;
#ifdef DEBUG
    // print out what state we're in
  //  std::cout << _printerStatus._state << std::endl; 
#endif

    if(_statusWriteFd >= 0)
    {
        // send status info out the PE status pipe
        lseek(_statusWriteFd, 0, SEEK_SET);
        write(_statusWriteFd, &_printerStatus, sizeof(struct PrinterStatus)); 
    }
}

/// Translate the event handler events into state machine events
void PrintEngine::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case MotorInterrupt:
            MotorCallback((unsigned char*)data);
            break;

        case DoorInterrupt:
            DoorCallback((char*)data);
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

/// Handle commands that have already been interpreted
void PrintEngine::Handle(Command command)
{
 #ifdef DEBUG
//    std::cout << "in PrintEngine::Handle command = " << 
//                 command << std::endl;
#endif       
    switch(command)
    {
        case Start:             
            SetNumLayers(PrintData::GetNumLayers());            
            // start a print 
            _pPrinterStateMachine->process_event(EvStartPrint());
            break;
            
        case Cancel:
            // cancel the print in progress
            _pPrinterStateMachine->process_event(EvCancel());
            break;
            
        case Pause:
            _pPrinterStateMachine->process_event(EvPause());
            break;
            
        case Resume:
            _pPrinterStateMachine->process_event(EvResume());
            break;
            
        case Reset:    
            // reset
            _pPrinterStateMachine->process_event(EvReset());
            break;
            
        case StartPauseOrResume:          
            // either start, pause, or resume, depending on current printer state
            _pPrinterStateMachine->StartPauseOrResume();
            break;
            
        case Test:
            // show a test pattern, regardless of whatever else we're doing,
            // since this command is for test & setup only
            _projector.ShowTestPattern();
            break;
        
        // none of these commands are handled directly by the print engine
        case GetStatus:
        case SetPrintData:
        case GetSetting:
        case SetSetting:
        case RestoreSetting:
        case GetLogs:
        case SetFirmware:    
            break;

        case Exit:
            // user requested program termination
            // tear down SDL first (to enable restarting it)
            _projector.TearDown();
            exit(0);
            
        default:
            Logger::LogError(LOG_WARNING, errno, UNKNOWN_COMMAND_INPUT, command);
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
            Logger::LogError(LOG_WARNING, errno, ENABLE_PULSE_TIMER_ERROR);
        else
            Logger::LogError(LOG_WARNING, errno, DISABLE_PULSE_TIMER_ERROR);
    }
}

/// Start the timer whose expiration signals the end of exposure for a layer
void PrintEngine::StartExposureTimer(int seconds)
{
    struct itimerspec timer1Value;
    
    timer1Value.it_value.tv_sec = seconds;
    timer1Value.it_value.tv_nsec = 0;
    timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    timer1Value.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_exposureTimerFD, 0, &timer1Value, NULL) == -1)
    {
        Logger::LogError(LOG_ERR, errno, EXPOSURE_TIMER_ERROR);  
        exit(-1);
    }
}

/// Clears the timer whose expiration signals the end of exposure for a layer
void PrintEngine::ClearExposureTimer()
{
    // setting a 0 as the time disarms the timer
    StartExposureTimer(0);
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
        Logger::LogError(LOG_ERR, errno, EXPOSURE_TIMER_ERROR);  
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

/// Set or clear the number of layers in the current print.  
/// Also resets the current layer number.
void PrintEngine::SetNumLayers(int numLayers)
{
    _printerStatus._numLayers = numLayers;
    // the number of layers should only be set before starting a print,
    // or when clearing it at the end or cancelling of a print
    _printerStatus._currentLayer = 0;
}

/// Increment the current layer number, load its image, and return the layer 
/// number.
int PrintEngine::NextLayer()
{
    ++_printerStatus._currentLayer;  
    if(!_projector.LoadImageForLayer(_printerStatus._currentLayer))
    {
        // if no image available, there's no point in proceeding
        Logger::LogError(LOG_ERR, errno, NO_IMAGE_FOR_LAYER, 
                _printerStatus._currentLayer);
        CancelPrint(); 
    }
    return(_printerStatus._currentLayer);
}

/// Returns true or false depending on whether or not the current print
/// has any more layers to be printed.
bool PrintEngine::NoMoreLayers()
{
    return _printerStatus._currentLayer >= _printerStatus._numLayers;
}

/// Sets or clears the estimated print time
void PrintEngine::SetEstimatedPrintTime(bool set)
{
    if(set)
    {
        // TODO: for more accurate estimated print time, may need to accept 
        // fractional value for SEPARATION_TIME_SEC
        _printerStatus._estimatedSecondsRemaining =  
                ( 1 + _printerStatus._numLayers - _printerStatus._currentLayer) * 
                (DEFAULT_EXPOSURE_TIME_SEC + SEPARATION_TIME_SEC);
    }
    else
    {
        // clear remaining time and current layer
        _printerStatus._estimatedSecondsRemaining = 0;
        _printerStatus._currentLayer = 0;
    }
#ifdef DEBUG
//    std::cout << "set est print time to " << 
//                 _printerStatus._estimatedSecondsRemaining << std::endl;
#endif    
}

/// Update the estimated time remaining for the print, on the assumption this 
/// is called once for every pulse
void PrintEngine::DecreaseEstimatedPrintTime(int amount)
{
    _printerStatus._estimatedSecondsRemaining -= amount;
    
 #ifdef DEBUG
//    if(amount > 1)
//        std::cout << "decreased est print time by " << amount  << std::endl;
#endif    
   
}

/// Translates interrupts from motor board into state machine events
void PrintEngine::MotorCallback(unsigned char* status)
{
#ifdef DEBUG
//    std::cout << "in MotorCallback status = " << 
//                 ((int)status) << 
//                 "at time = " <<
//                 GetMillis() << std::endl;
#endif    
    // forward the translated event, or pass it on to the state machine when
    // the translation requires knowledge of the current state
    switch(*status)
    {
        case ERROR_STATUS:
            HandleError(MOTOR_ERROR, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
            
        case SUCCESS:
            _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            Logger::LogError(LOG_WARNING, errno, UNKNOWN_MOTOR_STATUS, 
                             (int)status);
            break;
    }    
}

/// Translates door button interrupts into state machine events
void PrintEngine::DoorCallback(char* data)
{
    // TODO: make sure the polarity here matches the way the switch is wired up
    // (here we're assuming it's pulled high when open)
    if(*data == '1')
        _pPrinterStateMachine->process_event(EvDoorOpened());
    else
        _pPrinterStateMachine->process_event(EvDoorClosed());
}
     
/// Handles errors 
void PrintEngine::HandleError(const char* errorMsg, bool fatal)
{
    // log and print out the error
    Logger::LogError(fatal ? LOG_ERR : LOG_WARNING, errno, errorMsg);
    
    // Idle the state machine for fatal errors 
    if(fatal)
        _pPrinterStateMachine->process_event(EvError());
}

 
/// Send a single-character command to the motor board
void PrintEngine::SendMotorCommand(unsigned char command)
{
#ifdef DEBUG    
// std::cout << "sending motor command: " << 
//                 command << std::endl;
#endif  
    _pMotor->Write(MOTOR_COMMAND, command);
}

/// Cleans up from any print in progress
void PrintEngine::CancelPrint()
{
    StopMotor();
    // clear the number of layers
    SetNumLayers(0);
    // clear pulse & exposure timers
    EnablePulseTimer(false);
    ClearExposureTimer();
    Exposing::ClearPendingExposureInfo();
}

/// Tell the motor to stop (whether it's moving now or not), and clear the 
/// motor timeout timer.
void PrintEngine::StopMotor()
{
    SendMotorCommand(STOP_MOTOR_COMMAND);
    ClearMotorTimeoutTimer();  
}

/// Find the remaining exposure time (to the nearest second))
int PrintEngine::GetRemainingExposureTimeSec()
{
    struct itimerspec curr;
    int secs;

    if (timerfd_gettime(_exposureTimerFD, &curr) == -1)
    {
        Logger::LogError(LOG_ERR, errno, REMAINING_EXPOSURE_ERROR);  
        exit(-1);
    }

    secs = curr.it_value.tv_sec;
    if(curr.it_value.tv_nsec > 500000000)
        ++ secs;
    
    return secs;
}

/// Determines if the door is open or not
bool PrintEngine::DoorIsOpen()
{
    char GPIOInputValue[64], value;
    
    sprintf(GPIOInputValue, "/sys/class/gpio/gpio%d/value", DOOR_INTERRUPT_PIN);
    
    // Open the file descriptor for the door switch GPIO
    int fd = open(GPIOInputValue, O_RDONLY);
    if(fd < 0)
    {
        Logger::LogError(LOG_ERR, errno, GPIO_INPUT_ERROR, DOOR_INTERRUPT_PIN);
        exit(-1);
    }  
    
    read(fd, &value, 1);

    close(fd);

	return (value == '1');
}

/// Wraps Projector's ShowImage method and handles errors
void PrintEngine::ShowImage()
{
    if(!_projector.ShowImage())
    {
        Logger::LogError(LOG_ERR, errno, CANT_SHOW_IMAGE_FOR_LAYER, 
                         _printerStatus._currentLayer);
        CancelPrint();  
    }
    
}
 
/// Wraps Projector's ShowBlack method and handles errors
void PrintEngine::ShowBlack()
{
    if(!_projector.ShowBlack())
    {
        Logger::LogError(LOG_ERR, errno, CANT_SHOW_BLACK);
        _projector.SetPowered(false);
        CancelPrint();  
    }
}

