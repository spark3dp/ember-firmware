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
#include <algorithm>
#include <math.h>

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
#include <MotorController.h>

#define VIDEOFRAME__SEC         (1.0 / 60.0)
#define MILLIDEGREES_PER_REV    (360000.0)


/// The only public constructor.  'haveHardware' can only be false in debug
/// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware) :
_delayTimerFD(-1),
_exposureTimerFD(-1),
_motorTimeoutTimerFD(-1),
_temperatureTimerFD(-1),
_statusReadFD(-1),
_statusWriteFd(-1),
_haveHardware(haveHardware),
_homeUISubState(NoUISubState),
_invertDoorSwitch(false),
_temperature(-1.0),
_gotRotationInterrupt(false),
_alreadyOverheated(false),
_inspectionRequested(false),
_skipCalibration(false),
_remainingMotorTimeoutSec(0.0)
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
    
    _delayTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_delayTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(DelayTimerCreate));
        exit(-1);
    }
    
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
    
    // create the I2C device for the motor controller
    // use 0xFF as slave address for testing without actual boards
    // note, this must be defined before starting the state machine!
    _pMotor = new Motor(haveHardware ? MOTOR_SLAVE_ADDRESS : 0xFF); 
    
    // construct the state machine and tell it this print engine owns it
    _pPrinterStateMachine = new PrinterStateMachine(this);  

    _invertDoorSwitch = (SETTINGS.GetInt(HARDWARE_REV) == 0);
    
    _pThermometer = new Thermometer(haveHardware);
    
    _pProjector = new Projector(PROJECTOR_SLAVE_ADDRESS, I2C0_PORT);

    _pPrintData.reset(new PrintData());
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
   
    if (_statusReadFD >= 0)
        close(_statusReadFD);

    if (_statusWriteFd >= 0)
        close(_statusWriteFd);
    
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
/// enters the Initializing state (i.e. on startup and reset))
void PrintEngine::Initialize()
{
    ClearMotorTimeoutTimer();
    _remainingMotorTimeoutSec = 0.0;
    _printerStatus._state = InitializingState;
    _printerStatus._UISubState = NoUISubState;
    ClearHomeUISubState();
    _printerStatus._change = NoChange;    
    _printerStatus._currentLayer = 0;
    _printerStatus._estimatedSecondsRemaining = 0;
    ClearError();
    
    StartTemperatureTimer(TEMPERATURE_MEASUREMENT_INTERVAL_SEC);
    
    if(!_pMotor->Initialize())  
        HandleError(MotorError, true);
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
    unsigned char status;
    
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
           
        case DelayEnd:
            _pPrinterStateMachine->process_event(EvDelayEnded());
            break;
            
        case ExposureEnd:
            _pPrinterStateMachine->process_event(EvExposed());
            break;
            
        case MotorTimeout:
            HandleError(MotorTimeoutError, true, NULL, 
                                                    (int)*(unsigned char*)data);
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
    bool result;    
    switch(command)
    {
        case Start:          
            // start a print 
            _pPrinterStateMachine->process_event(EvStartPrint());
            break;
            
        case Cancel:
            // cancel any print in progress
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
            LogStatusAndSettings(); //for the record
            break;
            
        case ApplySettings:
            // load the settings for the printer or a print
            result = SETTINGS.SetFromFile(TEMP_SETTINGS_FILE);
            DeleteTempSettingsFile();
            if(!result)
                HandleError(CantLoadSettingsFile, true, TEMP_SETTINGS_FILE);
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

/// Start the timer used for various delays.
void PrintEngine::StartDelayTimer(double seconds)
{
    struct itimerspec timerValue;
    
    timerValue.it_value.tv_sec = (int)seconds;
    timerValue.it_value.tv_nsec = (int)(1E9 * 
                                       (seconds - timerValue.it_value.tv_sec));
    timerValue.it_interval.tv_sec =0; // don't automatically repeat
    timerValue.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_delayTimerFD, 0, &timerValue, NULL) == -1)
        HandleError(PreExposureDelayTimer, true);  
}

/// Clears the timer used for various delays 
void PrintEngine::ClearDelayTimer()
{
    // setting a 0 as the time disarms the timer
    StartDelayTimer(0.0);
}

/// Get the pre exposure delay time for the current layer
double PrintEngine::GetPreExposureDelayTimeSec()
{    
    // settings are in milliseconds
    return _cls.ApproachWaitMS / 1000.0;
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
    StartExposureTimer(0.0);
}

/// Get the exposure time for the current layer
double PrintEngine::GetExposureTimeSec()
{
    double expTime = _cls.ExposureSec;

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

/// Start the timer whose expiration indicates that the motor controller hasn't 
/// signaled its command completion in the expected time
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

/// Clears the timer whose expiration indicates that the motor controller hasn't 
/// signaled its command completion in the expected time
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

    SDL_Surface* image = _pPrintData->GetImageForLayer(_printerStatus._currentLayer);
    
    if(!image)
    {
        // if no image available, there's no point in proceeding
        HandleError(NoImageForLayer, true, NULL,
                    _printerStatus._currentLayer);
        ClearCurrentPrint(); 
    }
    else
    {
        // update projector with image
        _pProjector->SetImage(image);
        
        // log temperature at start, end, and quartile points
        int layer = _printerStatus._currentLayer;
        int total = _printerStatus._numLayers;
        if(layer == 1 || 
           layer == (int) (total * 0.25) || 
           layer == (int) (total * 0.5)  || 
           layer == (int) (total * 0.75) || 
           layer == total)
        {
        
        char msg[100];
        sprintf(msg, LOG_TEMPERATURE_PRINTING, layer, total, _temperature);
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
        double burnInTime = GetLayerTimeSec(BurnIn);
        double modelTime = GetLayerTimeSec(Model);
        double layerTimes = 0.0;
        
        // remaining time depends first on what kind of layer we're in
        if(IsFirstLayer())
        {
            layerTimes = GetLayerTimeSec(First) +
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
}

/// Tells state machine that an interrupt has arrived from the motor controller,
/// and whether or not the expected motion completed successfully.
void PrintEngine::MotorCallback(unsigned char* status)
{
    // clear the pending timeout
    ClearMotorTimeoutTimer();

#ifdef DEBUG
//    std::cout << "in MotorCallback status = " << 
//                 ((int)*status) << 
//                 " at time = " <<
//                 GetMillis() << std::endl;
#endif    
    switch(*status)
    {        
        case MC_STATUS_SUCCESS:
            _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            // any motor error is fatal
            HandleError(MotorControllerError, true, NULL, (int)*status);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
    }    
}

/// Tells the state machine to handle door sensor events
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

/// Send a high-level command to the motor controller 
/// (which may be translated into several low-level commands),
/// and set the timeout timer.
void PrintEngine::SendMotorCommand(int command)
{
#ifdef DEBUG    
// std::cout << "sending motor command: " << command << std::endl;
#endif  
    bool success = true;
        
    switch(command)
    {
        case HOME_COMMAND:
            success = _pMotor->GoHome();
            StartMotorTimeoutTimer(GetHomingTimeoutSec());
            break;
            
        case MOVE_TO_START_POSN_COMMAND: 
            success = _pMotor->GoToStartPosition();
            // for tracking where we are, to enable lifting for inspection
            _currentZPosition = 0;
            StartMotorTimeoutTimer(GetStartPositionTimeoutSec());
            break;
            
        case SEPARATE_COMMAND:
            success = _pMotor->Separate(_cls);
            StartMotorTimeoutTimer(GetSeparationTimeoutSec());
            break;
                        
        case APPROACH_COMMAND:
            success = _pMotor->Approach(_cls);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec());
            break;
            
        case APPROACH_AFTER_JAM_COMMAND:
            success = _pMotor->Approach(_cls, true);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec() +
                                   GetUnjammingTimeoutSec());
            break;
            
        case PRESS_COMMAND:
            success = _pMotor->Press(_cls);
            StartMotorTimeoutTimer(GetPressTimeoutSec());
            break;
            
         case UNPRESS_COMMAND:
            success = _pMotor->Unpress(_cls);
            StartMotorTimeoutTimer(GetUnpressTimeoutSec());
            break;
 
        case PAUSE_AND_INSPECT_COMMAND:
            success = _pMotor->PauseAndInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(true));
            break;
            
        case RESUME_FROM_INSPECT_COMMAND:
            success = _pMotor->ResumeFromInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(false));
            break;
            
        case JAM_RECOVERY_COMMAND:
            success = _pMotor->UnJam(_cls);
            StartMotorTimeoutTimer(GetUnjammingTimeoutSec());
            break;
            
        default:
            HandleError(UnknownMotorCommand, false, NULL, command);
            break;
    } 
    if(!success)    // the specific error was already logged
        HandleError(MotorError, true);
}

/// Cleans up from any print in progress.  If withInterrupt, and interrupt will 
/// be requested when clearing any pending movement, in case a movement is 
/// currently in progress.
void PrintEngine::ClearCurrentPrint(bool withInterrupt)
{
    PauseMovement();
    ClearPendingMovement(withInterrupt);
    
    // log the temperature, for canceled prints or on fatal error
    char msg[50];
    sprintf(msg, LOG_TEMPERATURE, _temperature);
    LOGGER.LogMessage(LOG_INFO, msg); 
    
    // clear the number of layers
    SetNumLayers(0);
    // clear timers
    ClearDelayTimer();
    ClearExposureTimer();
    Exposing::ClearPendingExposureInfo();
    _printerStatus._estimatedSecondsRemaining = 0;
    // clear pause & inspect flags
    _inspectionRequested = false;
}

/// Indicate that no print job is in progress
void PrintEngine::ClearJobID()
{
    SETTINGS.Set(JOB_ID_SETTING, std::string(""));
    SETTINGS.Save(); 
    
    // get a new unique ID for the next local job (if any)
    GetUUID(_printerStatus._localJobUniqueID); 
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
    return _pPrintData->GetNumLayers(SETTINGS.GetString(PRINT_DATA_DIR)) >= 1;
}

/// See if we can start a print, and if so perform the necessary initialization
bool PrintEngine::TryStartPrint()
{
    ClearError();            
    _skipCalibration = false;
            
    // make sure we have valid data
    std::string printDataDir = SETTINGS.GetString(PRINT_DATA_DIR);
    if(!_pPrintData->Validate(printDataDir))
    {
       HandleError(NoValidPrintDataAvailable, true); 
       return false;
    }
    
    SetNumLayers(_pPrintData->GetNumLayers(printDataDir));
    
    // use per-layer settings, if file defining them exists
    _perLayer.Load(printDataDir + PER_LAYER_SETTINGS_FILE);
    
    // make sure the temperature isn't too high to print
    if(IsPrinterTooHot())
        return false;
    
    // this would be a good point at which to validate print settings, 
    // if we knew the valid range for each

    // for the record:
    LogStatusAndSettings();
       
    ClearHomeUISubState();
     
    return true;
}

/// Show a screen related to print data when in the Home state
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

/// Prepare downloaded print data for printing.
void PrintEngine::ProcessData()
{
    // If any processing step fails, clear downloading screen, report an error,
    // and return to prevent any further processing
    
    if (!_pPrintData->Stage())
    {
        HandleDownloadFailed(PrintDataStageError, NULL);
        return;
    }

    if (!_pPrintData->Validate(SETTINGS.GetString(STAGING_DIR)))
    {
        HandleDownloadFailed(InvalidPrintData, _pPrintData->GetFileName().c_str());
        return;
    }

    bool settingsLoaded = _pPrintData->LoadSettings();
    DeleteTempSettingsFile();
    if (!settingsLoaded)
    {
        HandleDownloadFailed(PrintDataSettings, _pPrintData->GetFileName().c_str());
        return;
    }

    // At this point the incoming print data is sound so existing print data can be discarded
    if (!_pPrintData->Clear())
    {
        HandleDownloadFailed(PrintDataRemove, NULL);
        return;
    }

    if (!_pPrintData->MovePrintData())
    {
        // Set the jobName to empty string since the print data corresponding to
        // the jobName loaded with the settings has been removed
        SETTINGS.Set(JOB_NAME_SETTING, std::string(""));
        SETTINGS.Save();
        
        HandleDownloadFailed(PrintDataMove, _pPrintData->GetFileName().c_str());
        return;
    }

    // record the name of the last file downloaded
    SETTINGS.Set(PRINT_FILE_SETTING, _pPrintData->GetFileName());
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
    if(_pPrintData->Clear())
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
    if(access(TEMP_SETTINGS_FILE, F_OK) == 0)
        remove(TEMP_SETTINGS_FILE);
}


/// Gets the time (in seconds) required to print a layer based on the 
/// current settings for the type of layer.  Note: does not take into account
/// per-layer setting overrides that may change the actual print time.
double PrintEngine::GetLayerTimeSec(LayerType type)
{
    double time, press, revs, zLift;
    double height = SETTINGS.GetInt(LAYER_THICKNESS);
       
    switch(type)
    {
        case First:
            // start with the exposure time, in seconds
            time = (double) SETTINGS.GetDouble(FIRST_EXPOSURE);
            // plus additional delay (converted from ms)
            time += SETTINGS.GetInt(FL_APPROACH_WAIT) / 1000.0;
            // add separation time
            revs = SETTINGS.GetInt(FL_ROTATION) / MILLIDEGREES_PER_REV;
            // rotation speeds in RPM, convert to revs per sec
            time += (revs / SETTINGS.GetInt(FL_SEPARATION_R_SPEED)) * 60.0;
            // Z speeds are in microns/s
            zLift = SETTINGS.GetInt(FL_Z_LIFT);
            time += zLift / SETTINGS.GetInt(FL_SEPARATION_Z_SPEED);
            // add approach time
            time += (revs / SETTINGS.GetInt(FL_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / SETTINGS.GetInt(FL_APPROACH_Z_SPEED);
            // add press/delay/unpress times, if tray deflection used
            press = SETTINGS.GetInt(FL_PRESS);
            if(press != 0)
            {
                time += press / SETTINGS.GetInt(FL_PRESS_SPEED);
                time += SETTINGS.GetInt(FL_PRESS_WAIT) / 1000.0;
                time += press / SETTINGS.GetInt(FL_UNPRESS_SPEED);
            }
            break;
            
        case BurnIn:
            time = (double) SETTINGS.GetDouble(BURN_IN_EXPOSURE);
            time += SETTINGS.GetInt(BI_APPROACH_WAIT) / 1000.0;   
            revs = SETTINGS.GetInt(BI_ROTATION) / MILLIDEGREES_PER_REV;
            time += (revs / SETTINGS.GetInt(BI_SEPARATION_R_SPEED)) * 60.0;
            zLift = SETTINGS.GetInt(BI_Z_LIFT);
            time += zLift / SETTINGS.GetInt(BI_SEPARATION_Z_SPEED);
            time += (revs / SETTINGS.GetInt(BI_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / SETTINGS.GetInt(BI_APPROACH_Z_SPEED);
            press = SETTINGS.GetInt(BI_PRESS);
            if(press != 0)
            {
                time += press / SETTINGS.GetInt(BI_PRESS_SPEED);
                time += SETTINGS.GetInt(BI_PRESS_WAIT) / 1000.0;
                time += press / SETTINGS.GetInt(BI_UNPRESS_SPEED);
            }
            break;
            
        case Model:
            time = (double) SETTINGS.GetDouble(MODEL_EXPOSURE);
            time += SETTINGS.GetInt(ML_APPROACH_WAIT) / 1000.0;    
            revs = SETTINGS.GetInt(ML_ROTATION) / MILLIDEGREES_PER_REV;
            time += (revs / SETTINGS.GetInt(ML_SEPARATION_R_SPEED)) * 60.0;
            zLift = SETTINGS.GetInt(ML_Z_LIFT);
            time += zLift / SETTINGS.GetInt(ML_SEPARATION_Z_SPEED);
            time += (revs / SETTINGS.GetInt(ML_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / SETTINGS.GetInt(ML_APPROACH_Z_SPEED);
            press = SETTINGS.GetInt(ML_PRESS);
            if(press != 0)
            {
                time += press / SETTINGS.GetInt(ML_PRESS_SPEED);
                time += SETTINGS.GetInt(ML_PRESS_WAIT) / 1000.0;
                time += press / SETTINGS.GetInt(ML_UNPRESS_SPEED);
            }

            break; 
    }
    
    // add measured overhead 
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
    if(SETTINGS.GetInt(DETECT_JAMS) == 0 ||  // jam detection disabled or
       SETTINGS.GetInt(HARDWARE_REV) == 0)   // old hardware lacking this sensor
        return true; 
    
    return _gotRotationInterrupt;
}

/// Record whether or not a pause & inspect has been requested, 
/// and set UI sub-state if it has been requested.
void PrintEngine::SetInspectionRequested(bool requested) 
{
    _inspectionRequested = requested; 
    if(requested)
        SendStatus(_printerStatus._state, NoChange, AboutToPause);
}

/// Pause any movement in progress immediately (not a pause for inspection.)
void PrintEngine::PauseMovement()
{
    if(!_pMotor->Pause())   
        HandleError(MotorError, true);
    
    // pause the motor timeout timer too
    struct itimerspec curr;

    if (timerfd_gettime(_motorTimeoutTimerFD, &curr) == -1)
        HandleError(RemainingMotorTimeout, true);  

    _remainingMotorTimeoutSec = curr.it_value.tv_sec + 
                                curr.it_value.tv_nsec * 1E-9;

    ClearMotorTimeoutTimer();
}

/// Resume any paused movement in progress (not a resume from inspection.)
void PrintEngine::ResumeMovement()
{
    if(!_pMotor->Resume())
    {
        HandleError(MotorError, true);
        return;
    }
    
    if(_remainingMotorTimeoutSec > 0.0)
    {
        // resume the motor timeout timer too, for at least a second
        StartMotorTimeoutTimer((int)std::max(_remainingMotorTimeoutSec, 1.0));
        _remainingMotorTimeoutSec= 0.0;
    }
}

/// Abandon any movements still pending after a pause.
void PrintEngine::ClearPendingMovement(bool withInterrupt)
{
    if(!_pMotor->ClearPendingCommands(withInterrupt))  
        HandleError(MotorError, true);
    
    ClearMotorTimeoutTimer();
    _remainingMotorTimeoutSec= 0.0;
    
    if(withInterrupt)  // set timeout for awaiting completion of of the clear
        StartMotorTimeoutTimer((int)SETTINGS.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
}

/// Get the amount of tray deflection (if any) wanted after approach.
int PrintEngine::GetTrayDeflection()
{
    return _cls.PressMicrons;
}

/// Get the length of time to pause after tray deflection.
double PrintEngine::GetTrayDeflectionPauseTimeSec()
{
    // convert from milliseconds
    return _cls.PressWaitMS / 1000.0;
}

/// Pad the raw expected time for a movement to get a reasonable timeout period.
int  PrintEngine::PadTimeout(double rawTime)
{
    return (int) (rawTime * SETTINGS.GetDouble(MOTOR_TIMEOUT_FACTOR) + 
                            SETTINGS.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
}

/// Returns the timeout (in seconds) to allow for getting to the home position
int PrintEngine::GetHomingTimeoutSec()
{
    double rSpeed = SETTINGS.GetInt(R_HOMING_SPEED);
    double zSpeed = SETTINGS.GetInt(Z_HOMING_SPEED);

       
    double deltaR = 1;  // may take up to one full revolution
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    // Z height is in microns and speed in microns/s
    return PadTimeout(deltaR / rSpeed + 
                      abs(SETTINGS.GetInt(Z_START_PRINT_POSITION)) / zSpeed);   
}

/// Returns the timeout (in seconds) to allow for getting to the start position
int PrintEngine::GetStartPositionTimeoutSec()
{
    double rSpeed = SETTINGS.GetInt(R_START_PRINT_SPEED);
    double zSpeed = SETTINGS.GetInt(Z_START_PRINT_SPEED);

    double deltaR = SETTINGS.GetInt(R_START_PRINT_ANGLE);
    // convert to revolutions
    deltaR /= MILLIDEGREES_PER_REV;
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    // Z height is in microns and speed in microns/s
    
    return GetHomingTimeoutSec() +          // we also need to go home first
           PadTimeout(deltaR / rSpeed +  
                      abs(SETTINGS.GetInt(Z_START_PRINT_POSITION)) / zSpeed);   
}

/// Returns the timeout (in seconds) to allow for moving to or from the pause 
/// and inspect position.
int PrintEngine::GetPauseAndInspectTimeoutSec(bool toInspect)
{   
    double zSpeed, rSpeed;
    
    if(toInspect)
    {
        // moving up uses homing speeds
        rSpeed = SETTINGS.GetInt(R_HOMING_SPEED);
        zSpeed = SETTINGS.GetInt(Z_HOMING_SPEED);
    }
    else
    {
        // moving down uses start print speeds
        rSpeed = SETTINGS.GetInt(R_START_PRINT_SPEED);
        zSpeed = SETTINGS.GetInt(Z_START_PRINT_SPEED);
    }
      
    // convert to revolutions
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;
    // rSpeed is in RPM, convert to revolutions per second
    // Z height is in microns and speed in microns/s
    return PadTimeout((revs / rSpeed) * 60.0 +  
                      SETTINGS.GetInt(INSPECTION_HEIGHT) / zSpeed);
}

/// Returns the timeout (in seconds) to allow for attempting to recover from a
/// jam, which depends on the type of layer.
int PrintEngine::GetUnjammingTimeoutSec()
{   
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;

    // assume we may take twice as long as normal to get to the home position,
    // and then we also need to rotate back to the separation position
    revs *= 3.0; 
    // convert to revolutions per second
    return PadTimeout((revs / _cls.SeparationRPM) * 60.0);
}

/// Get the time required for the tray deflection movement.
int PrintEngine::GetPressTimeoutSec()
{
    return PadTimeout(_cls.PressMicrons / (double) _cls.PressMicronsPerSec);
}

/// Get the time required for moving back from tray deflection.
int PrintEngine::GetUnpressTimeoutSec()
{
    return PadTimeout(_cls.PressMicrons / (double) _cls.UnpressMicronsPerSec);
}

/// Gets the time required for separation from PDMS.  Assumes infinite jerk.
int PrintEngine::GetSeparationTimeoutSec()
{    
    // rotational speeds are in RPM
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;
    double time = (revs / _cls.SeparationRPM) * 60.0;
    
    // Z speeds are in microns/s
    time += _cls.ZLiftMicrons / (double) _cls.SeparationMicronsPerSec;
        
    return PadTimeout(time);   
}

/// Gets the time required for approach back to PDMS
int PrintEngine::GetApproachTimeoutSec()
{
    // rotational speeds are in RPM
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;
    double time = (revs / _cls.ApproachRPM) * 60.0;    
    
    // Z speeds are in microns/s
    time += (_cls.ZLiftMicrons - _cls.LayerThicknessMicrons) / 
                                            (double) _cls.ApproachMicronsPerSec;
            
    return PadTimeout(time);   
}

/// Read all of the settings applicable to the current layer into a struct
/// for reuse without having to look them up again.  This method should be 
/// called once per layer, before the layer number has been incremented 
/// for exposure.
void PrintEngine::GetCurrentLayerSettings()
{
    // Since we haven't incremented the layer number yet, the first settings 
    // (up through exposure) use the type and number of the next layer. 
    // The settings after exposure use that same layer type, but use the layer
    // number after that for any per-layer overrides.
    int n = GetCurrentLayerNum() + 1;
    int p = n + 1;
    
    // find the type of layer n
    LayerType type = Model;
    if(n == 1)
        type = First;
    else
    {
        int numBurnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
        if(numBurnInLayers > 0 && n <= 1 + numBurnInLayers)
            type = BurnIn;
    }

    switch(type)
    {
        case First:
            _cls.PressMicrons = _perLayer.GetInt(n, FL_PRESS);
            _cls.PressMicronsPerSec = _perLayer.GetInt(n, FL_PRESS_SPEED);
            _cls.PressWaitMS = _perLayer.GetInt(n, FL_PRESS_WAIT);
            _cls.UnpressMicronsPerSec = _perLayer.GetInt(n, FL_UNPRESS_SPEED);
            _cls.ApproachWaitMS = _perLayer.GetInt(n, FL_APPROACH_WAIT);
            _cls.ExposureSec = _perLayer.GetDouble(n, FIRST_EXPOSURE);
            
            _cls.SeparationRotJerk = _perLayer.GetInt(p, FL_SEPARATION_R_JERK);
            _cls.SeparationRPM = _perLayer.GetInt(p, FL_SEPARATION_R_SPEED);
            _cls.RotationMilliDegrees = _perLayer.GetInt(p, FL_ROTATION);
            _cls.SeparationZJerk = _perLayer.GetInt(p, FL_SEPARATION_Z_JERK);
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, FL_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, FL_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, FL_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, FL_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, FL_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, FL_APPROACH_Z_SPEED);
            break;
            
        case BurnIn:
            _cls.PressMicrons = _perLayer.GetInt(n, BI_PRESS);
            _cls.PressMicronsPerSec = _perLayer.GetInt(n, BI_PRESS_SPEED);
            _cls.PressWaitMS = _perLayer.GetInt(n, BI_PRESS_WAIT);
            _cls.UnpressMicronsPerSec = _perLayer.GetInt(n, BI_UNPRESS_SPEED);
            _cls.ApproachWaitMS = _perLayer.GetInt(n, BI_APPROACH_WAIT);
            _cls.ExposureSec = _perLayer.GetDouble(n, BURN_IN_EXPOSURE);
            
            _cls.SeparationRotJerk = _perLayer.GetInt(p, BI_SEPARATION_R_JERK);
            _cls.SeparationRPM = _perLayer.GetInt(p, BI_SEPARATION_R_SPEED);
            _cls.RotationMilliDegrees = _perLayer.GetInt(p, BI_ROTATION);
            _cls.SeparationZJerk = _perLayer.GetInt(p, BI_SEPARATION_Z_JERK);
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, BI_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, BI_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, BI_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, BI_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, BI_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, BI_APPROACH_Z_SPEED);
            break;
            
        case Model:
            _cls.PressMicrons = _perLayer.GetInt(n, ML_PRESS);
            _cls.PressMicronsPerSec = _perLayer.GetInt(n, ML_PRESS_SPEED);
            _cls.PressWaitMS = _perLayer.GetInt(n, ML_PRESS_WAIT);
            _cls.UnpressMicronsPerSec = _perLayer.GetInt(n, ML_UNPRESS_SPEED);
            _cls.ApproachWaitMS = _perLayer.GetInt(n, ML_APPROACH_WAIT);
            _cls.ExposureSec = _perLayer.GetDouble(n, MODEL_EXPOSURE); 
            
            _cls.SeparationRotJerk = _perLayer.GetInt(p, ML_SEPARATION_R_JERK);
            _cls.SeparationRPM = _perLayer.GetInt(p, ML_SEPARATION_R_SPEED);
            _cls.RotationMilliDegrees = _perLayer.GetInt(p, ML_ROTATION);
            _cls.SeparationZJerk = _perLayer.GetInt(p, ML_SEPARATION_Z_JERK);
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, ML_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, ML_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, ML_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, ML_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, ML_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, ML_APPROACH_Z_SPEED);
            break;
    }
    
    // likewise any layer thickness overrides come from the next layer
    _cls.LayerThicknessMicrons = _perLayer.GetInt(p, LAYER_THICKNESS);
    
    // to avoid changes while pause & inspect is already in progress:
    _cls.InspectionHeightMicrons = SETTINGS.GetInt(INSPECTION_HEIGHT);
    /// see if there's enough headroom to lift the model for inspection.
    _cls.CanInspect = (_cls.InspectionHeightMicrons != 0) && 
                      (SETTINGS.GetInt(MAX_Z_TRAVEL) > (_currentZPosition +  
                                                        _cls.ZLiftMicrons +
                                                _cls.InspectionHeightMicrons));
}

/// Indicate whether the last print is regarded as successful or failed.
void PrintEngine::SetPrintFeedback(PrintRating rating)
{
    _printerStatus._printRating = rating;
}