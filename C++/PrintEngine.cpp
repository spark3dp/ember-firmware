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
#include <PrintEngine.h>
#include <PrinterStateMachine.h>
#include <Logger.h>
#include <Filenames.h>
#include <PrintData.h>
#include <Settings.h>
#include <utils.h>
#include <Shared.h>
#include <MessageStrings.h>

#define VIDEOFRAME__SEC (1.0 / 60.0)


/// The only public constructor.  'haveHardware' can only be false in debug
/// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware) :
_exposureTimerFD(-1),
_motorTimeoutTimerFD(-1),
_temperatureTimerFD(-1),
_statusReadFD(-1),
_statusWriteFd(-1),
_awaitingMotorSettingAck(false),
_haveHardware(haveHardware),
_homeUISubState(NoUISubState),
_invertDoorSwitch(false),
_temperature(-1.0),
_cancelRequested(false),
_gotRotationInterrupt(false),
_alreadyOverheated(false),
_pauseRequested(false)
{
#ifndef DEBUG
    if(!haveHardware)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(HardwareNeeded));
        exit(-1);
    }
#endif  
    
    // the print engine "owns" its timers,
    //so it can enable and disable them as needed
    _exposureTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_exposureTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(ExposureTimerCreate));
        exit(-1);
    }
    
    _motorTimeoutTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_motorTimeoutTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(MotorTimerCreate));
        exit(-1);
    }

    _temperatureTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_temperatureTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(TemperatureTimerCreate));
        exit(-1);
    }
    
    // the print engine also "owns" the status update FIFO
    // don't recreate the FIFO if it exists already
    if (access(PRINTER_STATUS_PIPE, F_OK) == -1) {
        if (mkfifo(PRINTER_STATUS_PIPE, 0666) < 0) {
          LOGGER.LogError(LOG_ERR, errno, ERR_MSG(StatusPipeCreation));
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

    _invertDoorSwitch = (SETTINGS.GetInt(HARDWARE_REV) == 0);
    
    _pThermometer = new Thermometer(haveHardware);
    
    _pProjector = new Projector(PROJECTOR_SLAVE_ADDRESS, I2C0_PORT);
}

/// Destructor
PrintEngine::~PrintEngine()
{
    // the state machine apparently gets deleted without the following call, 
    // which therefore would cause an error
 //   delete _pPrinterStateMachine;
    
    delete _pMotor;
    delete _pThermometer;
    delete _pProjector;
    
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
    _printerStatus._state = InitializingState;
    _printerStatus._UISubState = NoUISubState;
    _printerStatus._change = NoChange;    
    _printerStatus._currentLayer = 0;
    _printerStatus._estimatedSecondsRemaining = 0;
    ClearError();
    
    StartTemperatureTimer(TEMPERATURE_MEASUREMENT_INTERVAL_SEC);
    
    // motor controller initialization could go here
}

/// Send out the status of the print engine, including current temperature
/// and status of any print in progress 
void PrintEngine::SendStatus(PrintEngineState state, StateChange change, 
                             UISubState substate)
{
    _printerStatus._state = state;
    _printerStatus._UISubState = substate;
    _printerStatus._change = change;
    _printerStatus._temperature = _temperature;

    if(_statusWriteFd >= 0)
    {
        // send status info out the PE status pipe
        write(_statusWriteFd, &_printerStatus, sizeof(struct PrinterStatus)); 
    }
}

/// Return the most recently set UI sub-state
UISubState PrintEngine::GetUISubState()
{
    return _printerStatus._UISubState;
}

/// Translate the event handler events into state machine events
void PrintEngine::Callback(EventType eventType, void* data)
{
    double exposureTimeLeft;
    
    switch(eventType)
    {
        case MotorInterrupt:
            MotorCallback((unsigned char*)data);
            break;
            
        case ButtonInterrupt:
            ButtonCallback((unsigned char*)data);
            break;

        case DoorInterrupt:
            DoorCallback((char*)data);
            break;
           
        case RotationInterrupt:
            _gotRotationInterrupt = true;
            break;
           
        case ExposureEnd:
            _pPrinterStateMachine->process_event(EvExposed());
            break;
            
        case MotorTimeout:
            HandleError(MotorTimeoutError, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
           
        case TemperatureTimer:
            // don't read thermometer while exposing, as it may reduce the
            // accuracy of the exposure time
            exposureTimeLeft = GetRemainingExposureTimeSec();
            if(exposureTimeLeft > 0.0)
                StartTemperatureTimer(exposureTimeLeft + 0.1);
            else
            {
                // read and record temperature
                _temperature = _pThermometer->GetTemperature();
#ifdef DEBUG
//                std::cout << "temperature = " << _temperature << std::endl;
#endif   
                if(!_alreadyOverheated)
                    IsPrinterTooHot();
                
                // keep reading temperature even if we're already overheated
                StartTemperatureTimer(TEMPERATURE_MEASUREMENT_INTERVAL_SEC);
            }
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
    PrintData printData;
    bool result;    
    switch(command)
    {
        case Start:          
            // start a print 
            _pPrinterStateMachine->process_event(EvStartPrint());
            break;
            
        case Cancel:
            // indicate that a print in progress should be canceled at the 
            // next opportunity
            _cancelRequested = true;
            // or cancel it now if current state handles this event
            _pPrinterStateMachine->process_event(EvCancel());
            break;
            
        case Pause:
            _pPrinterStateMachine->process_event(EvRequestPause());
            break;
            
        case Resume:
            _pPrinterStateMachine->process_event(EvResume());
            break;
            
        case Reset:    
            _pPrinterStateMachine->process_event(EvReset());
            break;
            
        case Test:           
            // show a test pattern, regardless of whatever else we're doing,
            // since this command is for test & setup only
            _pProjector->ShowTestPattern();                      
            break;
            
        case CalImage:           
            // show a calibration imagen, regardless of what we're doing,
            // since this command is for test & setup only
            _pProjector->ShowCalibrationPattern();                      
            break;
        
        case RefreshSettings:
            // reload the settings file
            SETTINGS.Refresh();
            break;
            
        case ApplyPrintSettings:
            // load the settings for a print
            result = printData.LoadSettings(TEMP_PRINT_SETTINGS_FILE);
            DeleteTempSettingsFile();
            if(!result)
                HandleError(CantLoadPrintSettingsFile, true, TEMP_PRINT_SETTINGS_FILE);
            break;
            
        case ShowPrintDataDownloading:
            ShowHomeScreenFor(DownloadingPrintData); 
            break;
            
        case ShowPrintDownloadFailed:
            ShowHomeScreenFor(PrintDownloadFailed); 
            break;
                
        case StartPrintDataLoad:
            ShowHomeScreenFor(LoadingPrintData); 
            break;
            
        case ProcessPrintData:
            ProcessData();
            break;
            
        case ShowPrintDataLoaded:
            ShowHomeScreenFor(LoadedPrintData);
            break;
            
        case StartRegistering:
            _pPrinterStateMachine->process_event(EvConnected());
            break;
            
        case RegistrationSucceeded:
            _homeUISubState = Registered;
            _pPrinterStateMachine->process_event(EvRegistered());
            break;
            
        case StartCalibration:
            _pPrinterStateMachine->process_event(EvStartCalibration());
            
        case ShowWiFiConnecting:
            ShowHomeScreenFor(WiFiConnecting);
            break;
            
        case ShowWiFiConnectionFailed:
            ShowHomeScreenFor(WiFiConnectionFailed);
            break;

        case ShowWiFiConnected:
            ShowHomeScreenFor(WiFiConnected);
            break;            
            
        case Exit:
            // user requested program termination
            // tear down SDL first (to enable restarting it)
            ExitHandler(0);
            break;
            
        default:
            HandleError(UnknownCommandInput, false, NULL, command); 
            break;
    }
}

/// Converts button events from UI board into state machine events
void PrintEngine::ButtonCallback(unsigned char* status)
{ 
        unsigned char maskedStatus = 0xF & (*status);
#ifdef DEBUG
//        std::cout << "button value = " << (int)*status  << std::endl;
//        std::cout << "button value after masking = " << (int)maskedStatus  << std::endl;
#endif    

    if(maskedStatus == 0)
    {
        // ignore any non-button events for now
        return;
    }
    
    // check for error status, in unmasked value
    if(*status == ERROR_STATUS)
    {
        HandleError(FrontPanelError);
        return;
    }
    
    // fire the state machine event corresponding to a button event
    switch(maskedStatus)
    {  
        case BTN1_PRESS:                    
            _pPrinterStateMachine->process_event(EvLeftButton());
            break;
            
        case BTN2_PRESS:          
            _pPrinterStateMachine->process_event(EvRightButton());
            break;
            
        case BTN1_HOLD:
            _pPrinterStateMachine->process_event(EvLeftButtonHold());
            break;  
            
        case BTN2_HOLD:
            _pPrinterStateMachine->process_event(EvRightButtonHold());
            break;
            
        case BTNS_1_AND_2_PRESS:
            // this case not currently used 
            break;            
                        
        default:
            HandleError(UnknownFrontPanelStatus, false, NULL, 
                                                                (int)*status);
            break;
    }        
}

/// Start the timer whose expiration signals the end of exposure for a layer
void PrintEngine::StartExposureTimer(double seconds)
{
    struct itimerspec timerValue;
    
    timerValue.it_value.tv_sec = (int)seconds;
    timerValue.it_value.tv_nsec = (int)(1E9 * 
                                       (seconds - timerValue.it_value.tv_sec));
    timerValue.it_interval.tv_sec =0; // don't automatically repeat
    timerValue.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_exposureTimerFD, 0, &timerValue, NULL) == -1)
        HandleError(ExposureTimer, true);  
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

    // actual exposure time includes an extra video frame, 
    // so reduce the requested time accordingly
    if(expTime > VIDEOFRAME__SEC)
        expTime -= VIDEOFRAME__SEC;
    
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

/// Returns the command to use for separation, 
/// which depends on the type of layer.
char PrintEngine::GetSeparationCommand()
{
    if(IsFirstLayer())
        return FIRST_SEPARATE_COMMAND;
    else if(IsBurnInLayer())
        return BURNIN_SEPARATE_COMMAND;
    else
        return MODEL_SEPARATE_COMMAND;   
}

/// Returns the timeout (in seconds) to allow for separation, 
/// which depends on the type of layer.
int PrintEngine::GetSeparationTimeoutSec()
{
    double timeoutSec = BASE_SEPARATION_MOTOR_TIMEOUT_SEC;
    
    if(IsFirstLayer())
        timeoutSec += GetLayerTime(First);
    else if(IsBurnInLayer())
        timeoutSec += GetLayerTime(BurnIn);
    else
        timeoutSec += GetLayerTime(Model);   

    timeoutSec -= GetExposureTimeSec();
    
    return (int)(timeoutSec + 0.5);
}

/// Start the timer whose expiration signals that the motor board has not 
// indicated that it's completed a command in the expected time
void PrintEngine::StartMotorTimeoutTimer(int seconds)
{
    struct itimerspec timerValue;
    
    timerValue.it_value.tv_sec = seconds;
    timerValue.it_value.tv_nsec = 0;
    timerValue.it_interval.tv_sec =0; // don't automatically repeat
    timerValue.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_motorTimeoutTimerFD, 0, &timerValue, NULL) == -1)
        HandleError(MotorTimeoutTimer, true);  
}

/// Start (or restart) the timer whose expiration signals that it's time to 
/// measure the temperature
void PrintEngine::StartTemperatureTimer(double seconds)
{
    struct itimerspec timerValue;
    
    timerValue.it_value.tv_sec = (int) seconds;
    timerValue.it_value.tv_nsec = (int)(1E9 * 
                                       (seconds - timerValue.it_value.tv_sec));
    timerValue.it_interval.tv_sec =0; // don't automatically repeat
    timerValue.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_temperatureTimerFD, 0, &timerValue, NULL) == -1)
        HandleError(TemperatureTimerError, true);  
}

/// Clears the timer whose expiration signals that the motor board has not 
// indicated that it's completed a command in the expected time
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

/// Increment the current layer number and attempt to load its image.  Returns
/// true only if that succeeds. Logs temperature on the quartiles.
bool PrintEngine::NextLayer()
{
    bool retVal = false;
    
    ++_printerStatus._currentLayer;  
    if(!_pProjector->LoadImageForLayer(_printerStatus._currentLayer))
    {
        // if no image available, there's no point in proceeding
        HandleError(NoImageForLayer, true, NULL,
                    _printerStatus._currentLayer);
        ClearCurrentPrint(); 
    }
    else  // log temperature at start, end, and quartile points
    {
        int layer = _printerStatus._currentLayer;
        int total = _printerStatus._numLayers;
        if(layer == 1 || 
           layer == (int) (total * 0.25) || 
           layer == (int) (total * 0.5)  || 
           layer == (int) (total * 0.75) || 
           layer == total)
        {
        
        char msg[50];
        sprintf(msg, LOG_TEMPERATURE, layer, total, _temperature);
        LOGGER.LogMessage(LOG_INFO, msg); 
        }
        retVal = true;
    }
    return retVal;
}

/// Returns true or false depending on whether or not the current print
/// has any more layers to be printed.
bool PrintEngine::NoMoreLayers()
{
    if(_printerStatus._currentLayer >= _printerStatus._numLayers)
    {
        // clear the print-in-progress status
        SetEstimatedPrintTime(false);
        return true;
    }
    else
        return false;
}

/// Sets or clears the estimated print time
void PrintEngine::SetEstimatedPrintTime(bool set)
{
    if(set)
    {
        int layersLeft = _printerStatus._numLayers - 
                        (_printerStatus._currentLayer - 1);
        
        double burnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
        double burnInTime = GetLayerTime(BurnIn);
        double modelTime = GetLayerTime(Model);
        double layerTimes = 0.0;
        
        // remaining time depends first on what kind of layer we're in
        if(IsFirstLayer())
        {
            layerTimes = GetLayerTime(First) +
                         burnInLayers * burnInTime + 
                         (_printerStatus._numLayers - (burnInLayers + 1)) * 
                                                                  modelTime;
        } 
        else if(IsBurnInLayer())
        {
            double burnInLayersLeft = burnInLayers - 
                                   (_printerStatus._currentLayer - 2);            
            double modelLayersLeft = layersLeft - burnInLayersLeft;
            
            layerTimes = burnInLayersLeft * burnInTime + 
                       modelLayersLeft  * modelTime;
            
        }
        else
        {
            // all the remaining layers are model layers
            layerTimes = layersLeft * modelTime;
        }
        
        _printerStatus._estimatedSecondsRemaining = (int)(layerTimes + 0.5);
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

/// Update the estimated time remaining for the print
void PrintEngine::DecreaseEstimatedPrintTime(double amount)
{
    _printerStatus._estimatedSecondsRemaining -= (int)(amount + 0.5);
    
 #ifdef DEBUG
//    if(amount + 0.5 > 1.0)
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
            HandleError(_awaitingMotorSettingAck ? MotorSettingError :
                                                   MotorError, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
            
        case SUCCESS:
            if(_awaitingMotorSettingAck)
            {
               _awaitingMotorSettingAck = false;
               _pPrinterStateMachine->process_event(EvGotSetting());
            }
            else
                _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            HandleError(UnknownMotorStatus, false, NULL, (int)*status);
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
    if(*data == (_invertDoorSwitch ? '1' : '0'))
        _pPrinterStateMachine->process_event(EvDoorClosed());
    else
        _pPrinterStateMachine->process_event(EvDoorOpened());
}
     
/// Handles errors with message and optional parameters
void PrintEngine::HandleError(ErrorCode code, bool fatal, 
                              const char* str, int value)
{
    char* msg;
    int origErrno = errno;
    // log the error
    const char* baseMsg = ERR_MSG(code);
    if(str != NULL)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                          str);
    else if (value != INT_MAX)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                        value);
    else
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg);
    
    // before setting any error codes into status:
    LogStatusAndSettings();
    
    // Report fatal errors and put the state machine in the Error state 
    if(fatal) 
    {
         // set the error into printer status
        _printerStatus._errorCode = code;
        _printerStatus._errno = origErrno;
        PrinterStatus::SetLastErrorMsg(msg);
        // indicate this is a new error
        _printerStatus._isError = true;
        // a status update will be sent when we enter the Error state
        _pPrinterStateMachine->HandleFatalError(); 
        // clear error status
        _printerStatus._isError = false;
    }
}

/// log firmware version, current print status, & current settings
void PrintEngine::LogStatusAndSettings()
{
    LOGGER.LogMessage(LOG_INFO, (std::string(FW_VERSION_MSG) + 
                                 GetFirmwareVersion()).c_str());
    LOGGER.LogMessage(LOG_INFO, _printerStatus.ToString().c_str());
    LOGGER.LogMessage(LOG_INFO, SETTINGS.GetAllSettingsAsJSONString().c_str());    
}

/// Clear the last error from printer status to be reported next
void PrintEngine::ClearError()
{
    _printerStatus._errorCode = Success;
    _printerStatus._errno = 0;
    PrinterStatus::SetLastErrorMsg("");
    // this flag should already be cleared, but just in case
    _printerStatus._isError = false; 
    
    // check for overheating again
    _alreadyOverheated = false;
}


/// Send a single-character command to the motor board
void PrintEngine::SendMotorCommand(unsigned char command)
{
#ifdef DEBUG    
// std::cout << "sending motor command: " << command << std::endl;
#endif  
    _pMotor->Write(MOTOR_COMMAND, command);
}

/// Format and send a multiple-character command string with the given value 
/// to the motor board
void PrintEngine::SendMotorCommand(const char* commandFormatString, int value)
{
    char buf[32];
    sprintf(buf, commandFormatString, value);
#ifdef DEBUG    
//    std::cout << "sending motor command: " << buf << std::endl;
#endif  
    _pMotor->Write(MOTOR_COMMAND, (const unsigned char*) buf, 
                                                    strlen((const char*)buf));
}

/// Cleans up from any print in progress
void PrintEngine::ClearCurrentPrint()
{
    // clear the number of layers
    SetNumLayers(0);
    // clear exposure timer
    ClearExposureTimer();
    Exposing::ClearPendingExposureInfo();
    _printerStatus._estimatedSecondsRemaining = 0;
    // clear pause & inspect flags
    _pPrinterStateMachine->_atInspectionPosition = false;
    _pauseRequested = false;
}

/// Indicate that no print job is in progress
void PrintEngine::ClearJobID()
{
    SETTINGS.Set(JOB_ID_SETTING, std::string(""));
    SETTINGS.Save();    
}
/// Find the remaining exposure time 
double PrintEngine::GetRemainingExposureTimeSec()
{
    struct itimerspec curr;

    if (timerfd_gettime(_exposureTimerFD, &curr) == -1)
        HandleError(RemainingExposure, true);  

    return curr.it_value.tv_sec + curr.it_value.tv_nsec * 1E-9;
}

/// Determines if the door is open or not
bool PrintEngine::DoorIsOpen()
{
    if(!_haveHardware)
        return false;
    
    char GPIOInputValue[64], value;
    
    sprintf(GPIOInputValue, GPIO_VALUE, DOOR_SENSOR_PIN);
    
    // Open the file descriptor for the door switch GPIO
    int fd = open(GPIOInputValue, O_RDONLY);
    if(fd < 0)
    {
        HandleError(GpioInput, true, NULL, DOOR_SENSOR_PIN);
        exit(-1);
    }  
    
    read(fd, &value, 1);

    close(fd);

	return (value == (_invertDoorSwitch ? '0' : '1'));
}

/// Wraps Projector's ShowImage method and handles errors
void PrintEngine::ShowImage()
{
    if(!_pProjector->ShowImage())
    {
        HandleError(CantShowImage, true, NULL, _printerStatus._currentLayer);
        ClearCurrentPrint();  
    }
    
}
 
/// Wraps Projector's ShowBlack method and handles errors
void PrintEngine::ShowBlack()
{
    if(!_pProjector->ShowBlack())
    {
        HandleError(CantShowBlack, true);
        ClearCurrentPrint();  
    }
}

/// Returns true if and only if there is at least one layer image present 
/// (though it/they may still not be valid for printing)
bool PrintEngine::HasAtLeastOneLayer()
{
    return PrintData::GetNumLayers(SETTINGS.GetString(PRINT_DATA_DIR)) >= 1;
}

/// See if we can start a print, and if so perform the necessary initialization
bool PrintEngine::TryStartPrint()
{
    ClearError();            
    _cancelRequested = false; 
            
    // make sure we have valid data
    std::string printDataDir = SETTINGS.GetString(PRINT_DATA_DIR);
    if(!PrintData::Validate(printDataDir))
    {
       HandleError(NoValidPrintDataAvailable, true); 
       return false;
    }
    
    SetNumLayers(PrintData::GetNumLayers(printDataDir));
    
    // make sure the temperature isn't too high to print
    if(IsPrinterTooHot())
        return false;
    
    // for the record:
    LogStatusAndSettings();
       
    // create the collection of settings to be sent to the motor board
    _motorSettings.clear();
    _motorSettings[LAYER_THICKNESS]       = LAYER_THICKNESS_COMMAND;
    
    _motorSettings[FL_SEPARATION_R_SPEED] = FL_SEPARATION_R_SPEED_COMMAND;
    _motorSettings[FL_APPROACH_R_SPEED]   = FL_APPROACH_R_SPEED_COMMAND;
    _motorSettings[FL_Z_LIFT]             = FL_Z_LIFT_COMMAND;
    _motorSettings[FL_SEPARATION_Z_SPEED] = FL_SEPARATION_Z_SPEED_COMMAND;
    _motorSettings[FL_APPROACH_Z_SPEED]   = FL_APPROACH_Z_SPEED_COMMAND;
    _motorSettings[FL_ROTATION]           = FL_ROTATION_COMMAND;
    _motorSettings[FL_EXPOSURE_WAIT]      = FL_EXPOSURE_WAIT_COMMAND;
    _motorSettings[FL_SEPARATION_WAIT]    = FL_SEPARATION_WAIT_COMMAND;
    _motorSettings[FL_APPROACH_WAIT]      = FL_APPROACH_WAIT_COMMAND;

    _motorSettings[BI_SEPARATION_R_SPEED] = BI_SEPARATION_R_SPEED_COMMAND;
    _motorSettings[BI_APPROACH_R_SPEED]   = BI_APPROACH_R_SPEED_COMMAND;
    _motorSettings[BI_Z_LIFT]             = BI_Z_LIFT_COMMAND;
    _motorSettings[BI_SEPARATION_Z_SPEED] = BI_SEPARATION_Z_SPEED_COMMAND;
    _motorSettings[BI_APPROACH_Z_SPEED]   = BI_APPROACH_Z_SPEED_COMMAND;
    _motorSettings[BI_ROTATION]           = BI_ROTATION_COMMAND;
    _motorSettings[BI_EXPOSURE_WAIT]      = BI_EXPOSURE_WAIT_COMMAND;
    _motorSettings[BI_SEPARATION_WAIT]    = BI_SEPARATION_WAIT_COMMAND;
    _motorSettings[BI_APPROACH_WAIT]      = BI_APPROACH_WAIT_COMMAND;

    _motorSettings[ML_SEPARATION_R_SPEED] = ML_SEPARATION_R_SPEED_COMMAND;
    _motorSettings[ML_APPROACH_R_SPEED]   = ML_APPROACH_R_SPEED_COMMAND;
    _motorSettings[ML_Z_LIFT]             = ML_Z_LIFT_COMMAND;
    _motorSettings[ML_SEPARATION_Z_SPEED] = ML_SEPARATION_Z_SPEED_COMMAND;
    _motorSettings[ML_APPROACH_Z_SPEED]   = ML_APPROACH_Z_SPEED_COMMAND;
    _motorSettings[ML_ROTATION]           = ML_ROTATION_COMMAND;
    _motorSettings[ML_EXPOSURE_WAIT]      = ML_EXPOSURE_WAIT_COMMAND;
    _motorSettings[ML_SEPARATION_WAIT]    = ML_SEPARATION_WAIT_COMMAND;
    _motorSettings[ML_APPROACH_WAIT]      = ML_APPROACH_WAIT_COMMAND;   
    
    ClearHomeUISubState();
    
#ifdef DEBUG
//    std::cout << "First layer time = " << GetLayerTime(First) << std::endl;
//    std::cout << "Burnin layer time = " << GetLayerTime(BurnIn) << std::endl;
//    std::cout << "Model layer time = " << GetLayerTime(Model) << std::endl;
#endif    
 
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
        
        // validate that the settings values are in range
        // when the format string includes "%0<number>d",
        // where <number> gives the max number of decimal digits allowed
        const char* precision = strstr(cmdString, "%0");
        if (precision != NULL)
        {
            int numDigits = atoi(precision +2); // start just past the "%0"
            int maxValue = (int)(pow(10, numDigits) - 1.0);
#ifdef DEBUG
//            std::cout << "For " << it->first << 
//                         " max value = " << maxValue << std::endl;
#endif    
            
            if(value < 0 || value > maxValue)
            {
                HandleError(SettingOutOfRange, true, it->first);
                // don't send this setting, and return false 
                // so that we won't start moving to the start position 
                return false;
            } 
        }
        
        // send the motor board command to set the setting
        _awaitingMotorSettingAck = true;
        SendMotorCommand(cmdString, value);
        return false;
    }
    else
    {
        // no more settings to be sent
        return true;
    }
}

// Show a screen related to print data when in the Home state
bool PrintEngine::ShowHomeScreenFor(UISubState substate)
{
   // These screens can only be shown in the Home state
    if (_printerStatus._state != HomeState)
    {
        HandleError(IllegalStateForUISubState, false, 
                                            STATE_NAME(_printerStatus._state));
        return false;
    }

    // Show the appropriate screen on the front panel  
    _homeUISubState = substate;
    SendStatus(_printerStatus._state, NoChange, substate);
    return true;
}

void PrintEngine::ProcessData()
{
    PrintData printData; 
    // If any processing step fails, clear downloading screen, report an error,
    // and return to prevent any further processing
    
    if (!printData.Stage())
    {
        HandleDownloadFailed(PrintDataStageError, NULL);
        return;
    }

    if (!printData.Validate(SETTINGS.GetString(STAGING_DIR)))
    {
        HandleDownloadFailed(InvalidPrintData, printData.GetFileName().c_str());
        return;
    }

    bool settingsLoaded = printData.LoadSettings();
    DeleteTempSettingsFile();
    if (!settingsLoaded)
    {
        HandleDownloadFailed(PrintDataSettings, printData.GetFileName().c_str());
        return;
    }

    // At this point the incoming print data is sound so existing print data can be discarded
    if (!printData.Clear())
    {
        HandleDownloadFailed(PrintDataRemove, NULL);
        return;
    }

    if (!printData.MovePrintData())
    {
        // Set the jobName to empty string since the print data corresponding to
        // the jobName loaded with the settings has been removed
        SETTINGS.Set(JOB_NAME_SETTING, std::string(""));
        SETTINGS.Save();
        
        HandleDownloadFailed(PrintDataMove, printData.GetFileName().c_str());
        return;
    }

    // record the name of the last file downloaded
    SETTINGS.Set(PRINT_FILE_SETTING, printData.GetFileName());
    SETTINGS.Save();
    
    ShowHomeScreenFor(LoadedPrintData);
}

/// Convenience method handles the error and sends status update with
/// UISubState needed to show that download failed on the front panel
/// (unless we're already showing an error)
void PrintEngine::HandleDownloadFailed(ErrorCode errorCode, const char* jobName)
{
    HandleError(errorCode, false, jobName);
    _homeUISubState = PrintDataLoadFailed;
    // don't send new status if we're already showing a fatal error
    if(_printerStatus._state != ErrorState)
        SendStatus(_printerStatus._state, NoChange, PrintDataLoadFailed);
}

/// Delete any existing printable data.
void PrintEngine::ClearPrintData()
{
    if(PrintData::Clear())
    {
        ClearHomeUISubState();
        // also clear job name, ID, and last print file
        std::string empty = "";
        SETTINGS.Set(JOB_NAME_SETTING, empty);
        SETTINGS.Set(PRINT_FILE_SETTING, empty);
        ClearJobID();   // also save settings changes
    }
    else
        HandleError(PrintDataRemove);        
}

/// Deletes the temporary settings file
void PrintEngine::DeleteTempSettingsFile()
{
    if(access(TEMP_PRINT_SETTINGS_FILE, F_OK) == 0)
        remove(TEMP_PRINT_SETTINGS_FILE);
}

/// Gets the time (in seconds) required to print a layer based on the 
/// current settings for the type of layer.
double PrintEngine::GetLayerTime(LayerType type)
{
    double time, revs, sepRSpeed, approachRSpeed, z, sepZSpeed, approachZSpeed;
    
    switch(type)
    {
        case First:
            // start with the exposure time, in seconds
            time = (double) SETTINGS.GetDouble(FIRST_EXPOSURE);
            // plus additional delays in ms
            time += (SETTINGS.GetInt(FL_EXPOSURE_WAIT) +
                     SETTINGS.GetInt(FL_SEPARATION_WAIT) + 
                     SETTINGS.GetInt(FL_APPROACH_WAIT)) / 1000.0;    
            // convert the angle of rotation in degrees/1000 to 
            // fractional revolutions
            revs = SETTINGS.GetInt(FL_ROTATION) / 360000.0;
            sepRSpeed = SETTINGS.GetInt(FL_SEPARATION_R_SPEED);
            approachRSpeed = SETTINGS.GetInt(FL_APPROACH_R_SPEED);
            // Z distances are in microns
            z = (double) SETTINGS.GetInt(FL_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(FL_SEPARATION_Z_SPEED);
            approachZSpeed =  SETTINGS.GetInt(FL_APPROACH_Z_SPEED);
            break;
            
        case BurnIn:
            time = (double) SETTINGS.GetDouble(BURN_IN_EXPOSURE);
            time += (SETTINGS.GetInt(BI_EXPOSURE_WAIT) +
                     SETTINGS.GetInt(BI_SEPARATION_WAIT) + 
                     SETTINGS.GetInt(BI_APPROACH_WAIT)) / 1000.0;    
            revs = SETTINGS.GetInt(BI_ROTATION) / 360000.0;
            sepRSpeed = SETTINGS.GetInt(BI_SEPARATION_R_SPEED);
            approachRSpeed = SETTINGS.GetInt(BI_APPROACH_R_SPEED);
            z = (double) SETTINGS.GetInt(BI_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(BI_SEPARATION_Z_SPEED);
            approachZSpeed =  SETTINGS.GetInt(BI_APPROACH_Z_SPEED);
            break;
            
        case Model:
            time = (double) SETTINGS.GetDouble(MODEL_EXPOSURE);
            time += (SETTINGS.GetInt(ML_EXPOSURE_WAIT) +
                     SETTINGS.GetInt(ML_SEPARATION_WAIT) + 
                     SETTINGS.GetInt(ML_APPROACH_WAIT)) / 1000.0;    
            revs = SETTINGS.GetInt(ML_ROTATION) / 360000.0;
            sepRSpeed = SETTINGS.GetInt(ML_SEPARATION_R_SPEED);
            approachRSpeed = SETTINGS.GetInt(ML_APPROACH_R_SPEED);
            z = (double) SETTINGS.GetInt(ML_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(ML_SEPARATION_Z_SPEED);
            approachZSpeed =  SETTINGS.GetInt(ML_APPROACH_Z_SPEED);
            break; 
    }
    
    // rotational speeds are in RPM
    revs *= 60;
    time += revs / sepRSpeed + revs / approachRSpeed;
    
    // Z speeds are in microns/s
    time += z / sepZSpeed +
           (z - SETTINGS.GetInt(LAYER_THICKNESS)) / approachZSpeed;
    
    // add overhead, not otherwise accounted for 
    time += SETTINGS.GetDouble(LAYER_OVERHEAD);
    
    return time;   
}

/// Checks to see if the printer is too hot to function
bool PrintEngine::IsPrinterTooHot()
{
    _alreadyOverheated = false;
    if(_temperature > SETTINGS.GetDouble(MAX_TEMPERATURE))
    {
        char val[20];
        sprintf(val, "%g", _temperature);
        HandleError(OverHeated, true, val);
        _alreadyOverheated =  true;
    }
    
    return _alreadyOverheated;
}

/// Check to see if we got the expected interrupt from the rotation sensor.
bool PrintEngine::GotRotationInterrupt()
{ 
    if(SETTINGS.GetInt(HARDWARE_REV) == 0)
        return true; // older hardware lacked this sensor
    
    return _gotRotationInterrupt;
}

/// Determines whether there's enough headroom to lift the model up for 
/// inspection.
bool PrintEngine::CanInspect()
{
    // get the amount of overlift for the current layer
    int overlift = SETTINGS.GetInt(ML_Z_LIFT);
    if(IsFirstLayer())
        overlift = SETTINGS.GetInt(FL_Z_LIFT);
    else if(IsBurnInLayer())
        overlift = SETTINGS.GetInt(BI_Z_LIFT);
    
    return SETTINGS.GetInt(MAX_Z_TRAVEL) > 
            (GetCurrentLayer() * SETTINGS.GetInt(LAYER_THICKNESS) +  overlift +
            SETTINGS.GetInt(INSPECTION_HEIGHT));
}

/// Get the amount of rotation (in thousandths of a degree) to be used when
/// pausing.
int PrintEngine::GetPauseRotation()
{
    int rotation; 
    
    if(IsFirstLayer())
        rotation = SETTINGS.GetInt(FL_ROTATION);
    else if(IsBurnInLayer())
        rotation = SETTINGS.GetInt(BI_ROTATION);
    else
        rotation = SETTINGS.GetInt(ML_ROTATION);
    
    return rotation;    
}

/// Record whether or not a pause has been requested, and set UI sub-state if
/// pause has been requested.
void PrintEngine::SetPauseRequested(bool requested) 
{
    _pauseRequested = requested; 
    if(requested)
        SendStatus(_printerStatus._state, NoChange, AboutToPause);
}
