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
#include <Settings.h>
#include <utils.h>


/// The only public constructor.  'haveHardware' can only be false in debug
/// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware) :
_pulseTimerFD(-1),
_pulsePeriodSec(PULSE_PERIOD_SEC),
_exposureTimerFD(-1),
_motorTimeoutTimerFD(-1),
_statusReadFD(-1),
_statusWriteFd(-1),
_awaitingMotorSettingAck(false)
{
#ifndef DEBUG
    if(!haveHardware)
    {
        LOGGER.LogError(LOG_ERR, errno, HARDWARE_NEEDED_ERROR);
        exit(-1);
    }
#endif  
    
    // the print engine "owns" its timers,
    //so it can enable and disable them as needed
    _pulseTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_pulseTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, PULSE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _exposureTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_exposureTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, EXPOSURE_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    _motorTimeoutTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_motorTimeoutTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, MOTOR_TIMER_CREATE_ERROR);
        exit(-1);
    }
    
    // the print engine also "owns" the status update FIFO
    // don't recreate the FIFO if it exists already
    if (access(PRINTER_STATUS_PIPE, F_OK) == -1) {
        if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0) {
          LOGGER.LogError(LOG_ERR, errno, STATUS_PIPE_CREATION_ERROR);
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
    ClearMotorTimeoutTimer();
    // TODO: more complete initialization of printer status
    _printerStatus._state = "undefined";
    _printerStatus._currentLayer = 0;
    _printerStatus._estimatedSecondsRemaining = 0;
    ClearError();
    
    // initialize the motor board
    
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
            ClearError();            
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
        
        case RefreshSettings:
            // reload the settings file
            SETTINGS.Refresh();
            break;
            
        // none of these commands are handled directly by the print engine
        // (or at least not yet in some cases)
        case GetStatus:
        case SetPrintData:
        case GetSetting:
        case SetSetting:
        case RestoreSetting:
        case GetLogs:
        case SetFirmware:
        case GetFWVersion:
        case GetBoardNum:
            break;

        case Exit:
            // user requested program termination
            // tear down SDL first (to enable restarting it)
            _projector.TearDown();
            exit(0);
            
        default:
            HandleError(UNKNOWN_COMMAND_INPUT, false, NULL, command); 
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
            HandleError(ENABLE_PULSE_TIMER_ERROR);
        else
            HandleError(DISABLE_PULSE_TIMER_ERROR);
    }
}

/// Start the timer whose expiration signals the end of exposure for a layer
void PrintEngine::StartExposureTimer(double seconds)
{
    struct itimerspec timer1Value;
    
    timer1Value.it_value.tv_sec = (int)seconds;
    timer1Value.it_value.tv_nsec = (int)( 1E9 * 
                                       (seconds - timer1Value.it_value.tv_sec));
    timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    timer1Value.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_exposureTimerFD, 0, &timer1Value, NULL) == -1)
        HandleError(EXPOSURE_TIMER_ERROR, true);  
}

/// Clears the timer whose expiration signals the end of exposure for a layer
void PrintEngine::ClearExposureTimer()
{
    // setting a 0 as the time disarms the timer
    StartExposureTimer(0);
}

/// Get the exposure time for the current layer
double PrintEngine::GetExposureTimeSec()
{
    double expTime = 0.0;
    if(IsFirstLayer())
    {
        // exposure time for first layer
        expTime = SETTINGS.GetDouble(FIRST_EXPOSURE);
    }
    else if (IsBurnInLayer())
    {
        // exposure time for burn-in layers
        expTime = SETTINGS.GetDouble(BURN_IN_EXPOSURE);
    }
    else
    {
        // exposure time for ordinary model layers
        expTime = SETTINGS.GetDouble(MODEL_EXPOSURE);
    }

    return expTime;
}

/// Returns true if and only if the current layer is the first one
bool PrintEngine::IsFirstLayer()
{
    return _printerStatus._currentLayer == 1;
}

/// Returns true if and only if the current layer is a burn-in layer
bool PrintEngine::IsBurnInLayer()
{
    int numBurnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
    return (numBurnInLayers > 0 && 
            _printerStatus._currentLayer > 1 &&
            _printerStatus._currentLayer <= 1 + numBurnInLayers);
}


/// Start the timer whose expiration signals that the motor board has not 
// indicated that it's completed a command in the expected time
void PrintEngine::StartMotorTimeoutTimer(int seconds)
{
    struct itimerspec timer1Value;
    
    timer1Value.it_value.tv_sec = seconds;
    timer1Value.it_value.tv_nsec = 0;
    timer1Value.it_interval.tv_sec =0; // don't automatically repeat
    timer1Value.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_motorTimeoutTimerFD, 0, &timer1Value, NULL) == -1)
        HandleError(MOTOR_TIMEOUT_TIMER_ERROR, true);  
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
    // or when clearing it at the end or canceling of a print
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
        HandleError(NO_IMAGE_FOR_LAYER, true, NULL,
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
        int layersLeft = _printerStatus._numLayers - 
                        (_printerStatus._currentLayer - 1);
        // first calculate the time needed between each exposure, for separation
        double sepTimes = layersLeft * SEPARATION_TIME_SEC;
        
        double burnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
        double burnInExposure = SETTINGS.GetDouble(BURN_IN_EXPOSURE);
        double modelExposure = SETTINGS.GetDouble(MODEL_EXPOSURE);
        double expTimes = 0.0;
        
        // remaining time depends first on what kind of layer we're in
        if(IsFirstLayer())
        {
            expTimes = SETTINGS.GetDouble(FIRST_EXPOSURE) + 
                       burnInLayers * burnInExposure + 
                       (_printerStatus._numLayers - (burnInLayers + 1)) * 
                                                                  modelExposure;
        } 
        else if(IsBurnInLayer())
        {
            double burnInLayersLeft = burnInLayers - 
                                   (_printerStatus._currentLayer - 2);            
            double modelLayersLeft = layersLeft - burnInLayersLeft;
            
            expTimes = burnInLayersLeft * burnInExposure + 
                       modelLayersLeft  * modelExposure;
            
        }
        else
        {
            // all the remaining layers are model layers
            expTimes = layersLeft * modelExposure;
        }
        
        _printerStatus._estimatedSecondsRemaining =
                                             (int)(expTimes + sepTimes + 0.5);
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
//                 ((int)*status) << 
//                 " at time = " <<
//                 GetMillis() << std::endl;
#endif    
    // forward the translated event, or pass it on to the state machine when
    // the translation requires knowledge of the current state
    switch(*status)
    {
        case ERROR_STATUS:
            // TODO: add special handling of error when _awaitingMotorSettingAck
            HandleError(MOTOR_ERROR, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
            
        case SUCCESS:
            // TODO: we'll want special status for 'setting' command completed,
            // that doesn't require motor movement and therefore a state change,
            // bot for now, handle it here. 
            if(_awaitingMotorSettingAck)
            {
               _awaitingMotorSettingAck = false;
               _pPrinterStateMachine->process_event(EvGotSetting());
            }
            else
                _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            HandleError(UNKNOWN_MOTOR_STATUS, false, NULL, (intptr_t)status);
            break;
    }    
}

/// Translates door button interrupts into state machine events
void PrintEngine::DoorCallback(char* data)
{
#ifdef DEBUG
//    std::cout << "in DoorCallback status = " << 
//                 *data << 
//                 " at time = " <<
//                 GetMillis() << std::endl;
#endif       
    if(*data == '1')
        _pPrinterStateMachine->process_event(EvDoorClosed());
    else
        _pPrinterStateMachine->process_event(EvDoorOpened());
}
     
/// Handles errors with message and optional parameters
void PrintEngine::HandleError(const char* baseMsg, bool fatal, 
                              const char* str, int value)
{
    char* msg;
    int origErrno = errno;
    // log and print out the error
    if(str != NULL)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                          str);
    else if (value != INT_MAX)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                        value);
    else
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg);
    
    // set the error message and number into printer status
    _printerStatus._errorMessage = msg;
    _printerStatus._errorCode = origErrno;
    // indicate this is a new error
    _printerStatus._isError = true;
    
    // report the error
    SendStatus(_printerStatus._state, NoChange);
    // clear error status
    _printerStatus._isError = false;
    
    // Idle the state machine for fatal errors 
    if(fatal)
            _pPrinterStateMachine->PostEvent(EvError());       
}


/// Clear the last error from printer status to be reported next;
void PrintEngine::ClearError()
{
    _printerStatus._errorMessage = "";
    _printerStatus._errorCode = 0;
    // this flag should already be cleared, but just in case
    _printerStatus._isError = false;    
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

/// Send a multiple-character command string to the motor board
void PrintEngine::SendMotorCommand(const unsigned char* commandString)
{
#ifdef DEBUG    
// std::cout << "sending motor command: " << 
//                 commandString << std::endl;
#endif  
    _pMotor->Write(MOTOR_COMMAND, commandString);
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
        HandleError(REMAINING_EXPOSURE_ERROR, true);  

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
        HandleError(GPIO_INPUT_ERROR, true, NULL, DOOR_INTERRUPT_PIN);
        exit(-1);
    }  
    
    read(fd, &value, 1);

    close(fd);

	return (value == '0');
}

/// Wraps Projector's ShowImage method and handles errors
void PrintEngine::ShowImage()
{
    if(!_projector.ShowImage())
    {
        HandleError(CANT_SHOW_IMAGE_FOR_LAYER, true, NULL, 
                    _printerStatus._currentLayer);
        CancelPrint();  
    }
    
}
 
/// Wraps Projector's ShowBlack method and handles errors
void PrintEngine::ShowBlack()
{
    if(!_projector.ShowBlack())
    {
        HandleError(CANT_SHOW_BLACK, true);
        _projector.SetPowered(false);
        CancelPrint();  
    }
}

/// See if we can start a print, and if so perform the necessary initialization
bool PrintEngine::TryStartPrint()
{
    // do we have valid data?
    if(PrintData::GetNumLayers() < 1)
    {
       HandleError(NO_PRINT_DATA, false); 
       return false;
    }
    
    // TODO: check for low-enough temperature and any other required conditions
    // and log error and return false if not met
    
    // log all settings being used for this print
    std::string msg = SETTINGS.GetAllSettingsAsJSONString();
    // replace newlines with spaces, so it can be on one line of the logs    
    LOGGER.LogMessage(LOG_INFO, Replace(msg, "\n", " ").c_str());
    
    _printerStatus._jobName = SETTINGS.GetString(JOB_NAME_SETTING).c_str();
    
    // create the collection of settings to be sent to the motor board
    _motorSettings.clear();
    _motorSettings[LAYER_THICKNESS] = LAYER_THICKNESS_COMMAND;
    _motorSettings[SEPARATION_RPM] = SEPARATION_RPM_COMMAND;
 
    return true;
}

/// Send any motor board settings needed for this print.
bool PrintEngine::SendSettings()
{
    // if there are settings in the list, send them and return false
    if(_motorSettings.size() > 0)
    {
        // get the first setting from the collection, and its command string
        std::map<const char*, const char*>::iterator it = _motorSettings.begin();
        int value = SETTINGS.GetInt(it->first);
        
        const char* cmdString = it->second;
        // remove that setting from the collection
        _motorSettings.erase(it);
        
        if(strcmp(it->first, SEPARATION_RPM) == 0)
        {
            // validate that the value is in range
            if(value < 0 || value > 9)
            {
                HandleError(SEPARATION_RPM_OUT_OF_RANGE, false, NULL, value);
                // don't send this setting, and return true 
                // if there are no more settings needing to be sent
                return _motorSettings.size() == 0;
            } 
        }
        
        // send the motor board command to set the setting
        char buf[32];
        sprintf(buf, cmdString, value);
        _awaitingMotorSettingAck = true;
        SendMotorCommand((const unsigned char*)buf);
        return false;
    }
    else
    {
        // no more settings to be sent
        return true;
    }
}

