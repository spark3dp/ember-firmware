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

#define VIDEOFRAME__SEC (1.0 / 60.0)

/// The only public constructor.  'haveHardware' can only be false in debug
/// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware) :
_preExposureDelayTimerFD(-1),
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
    
    _preExposureDelayTimerFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK); 
    if (_preExposureDelayTimerFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(PreExposureDelayTimerCreate));
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
           
        case PreExposureDelayEnd:
            _pPrinterStateMachine->process_event(EvDelayEnded());
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

/// Start the timer whose expiration signals the resin tray has settled,
/// so the exposure of a layer may begin
void PrintEngine::StartPreExposureDelayTimer(double seconds)
{
    struct itimerspec timerValue;
    
    timerValue.it_value.tv_sec = (int)seconds;
    timerValue.it_value.tv_nsec = (int)(1E9 * 
                                       (seconds - timerValue.it_value.tv_sec));
    timerValue.it_interval.tv_sec =0; // don't automatically repeat
    timerValue.it_interval.tv_nsec =0;
       
    // set relative timer
    if (timerfd_settime(_preExposureDelayTimerFD, 0, &timerValue, NULL) == -1)
        HandleError(PreExposureDelayTimer, true);  
}

/// Clears the timer whose expiration signals the end of delay before exposure 
void PrintEngine::ClearPreExposureDelayTimer()
{
    // setting a 0 as the time disarms the timer
    StartPreExposureDelayTimer(0.0);
}

/// Get the pre exposure delay time for the current layer
double PrintEngine::GetPreExposureDelayTimeSec()
{
    double expTime = 0.0;
    if(IsFirstLayer())
    {
        // exposure time for first layer
        expTime = SETTINGS.GetInt(FL_APPROACH_WAIT);
    }
    else if (IsBurnInLayer())
    {
        // exposure time for burn-in layers
        expTime = SETTINGS.GetDouble(BI_APPROACH_WAIT);
    }
    else
    {
        // exposure time for ordinary model layers
        expTime = SETTINGS.GetDouble(ML_APPROACH_WAIT);
    }
    
    // settings are in ms
    return expTime / 1000.0;
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
    double expTime = 0.0;
    int layer = _printerStatus._currentLayer;
    
    if(IsFirstLayer())
    {
        // exposure time for first layer
        expTime = _layerSettings.GetDouble(layer, FIRST_EXPOSURE);
    }
    else if (IsBurnInLayer())
    {
        // exposure time for burn-in layers
        expTime = _layerSettings.GetDouble(layer, BURN_IN_EXPOSURE);
    }
    else
    {
        // exposure time for ordinary model layers
        expTime = _layerSettings.GetDouble(layer, MODEL_EXPOSURE);
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

/// Gets the type (first, burn-in, or model) of the current layer
LayerType PrintEngine::GetCurrentLayerType()
{
    if(IsFirstLayer())
        return First;
    else if(IsBurnInLayer())
        return BurnIn;
    else
        return Model;    
}
/// Returns the timeout (in seconds) to allow for separation, 
/// which depends on the type of layer.
int PrintEngine::GetSeparationTimeoutSec()
{
    double timeoutSec = BASE_MOTOR_TIMEOUT_SEC;
    
    if(IsFirstLayer())
        timeoutSec += GetSeparationTimeSec(First);
    else if(IsBurnInLayer())
        timeoutSec += GetSeparationTimeSec(BurnIn);
    else
        timeoutSec += GetSeparationTimeSec(Model);   

    return (int)(timeoutSec + 0.5);
}

/// Returns the timeout (in seconds) to allow for approach, 
/// which depends on the type of layer.
int PrintEngine::GetApproachTimeoutSec()
{
    double timeoutSec = BASE_MOTOR_TIMEOUT_SEC;
    
    if(IsFirstLayer())
        timeoutSec += GetApproachTimeSec(First);
    else if(IsBurnInLayer())
        timeoutSec += GetApproachTimeSec(BurnIn);
    else
        timeoutSec += GetApproachTimeSec(Model);   
    
    return (int)(timeoutSec + 0.5);
}

/// Returns the timeout (in seconds) to allow for moving to or from the pause 
/// and inspect position, which depends on the type of layer.
int PrintEngine::GetPauseAndInspectTimeoutSec()
{   
    double zSpeed, rSpeed;
    
    if(IsFirstLayer())
    {
        rSpeed = SETTINGS.GetInt(FL_APPROACH_R_SPEED);
        zSpeed = SETTINGS.GetInt(FL_APPROACH_Z_SPEED);
    }
    else if(IsBurnInLayer())
    {
        rSpeed = SETTINGS.GetInt(BI_APPROACH_R_SPEED);
        zSpeed = SETTINGS.GetInt(BI_APPROACH_Z_SPEED);    
    }
    else
    {
        rSpeed = SETTINGS.GetInt(ML_APPROACH_R_SPEED);
        zSpeed = SETTINGS.GetInt(ML_APPROACH_Z_SPEED);    
    }
   
    double deltaR = GetInspectRotation();
    // convert to revolutions
    deltaR /= UNITS_PER_REVOLUTION * R_SCALE_FACTOR;
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    // Z height is in microns and speed in microns/s
    return (int)((deltaR / rSpeed) +  
                 (SETTINGS.GetInt(INSPECTION_HEIGHT) / zSpeed) +
                 BASE_MOTOR_TIMEOUT_SEC + 0.5);
}

/// Returns the timeout (in seconds) to allow for attempting to recover from a
/// jam, which depends on the type of layer.
int PrintEngine::GetUnjammingTimeoutSec()
{   
    double rotation, rSpeed;
    
    if(IsFirstLayer())
    {
        rSpeed = SETTINGS.GetInt(FL_SEPARATION_R_SPEED);
        rotation = SETTINGS.GetInt(FL_ROTATION);
    }
    else if(IsBurnInLayer())
    {
        rSpeed = SETTINGS.GetInt(BI_SEPARATION_R_SPEED);
        rotation = SETTINGS.GetInt(BI_ROTATION);
    }
    else
    {
        rSpeed = SETTINGS.GetInt(ML_SEPARATION_R_SPEED);
        rotation = SETTINGS.GetInt(ML_ROTATION);
    }

    // assume we may take twice as long as normal to get to the home position,
    // and then we also need to rotate back to the separation position
    rotation *= 3.0; 
    // convert to revolutions
    rotation /= UNITS_PER_REVOLUTION * R_SCALE_FACTOR;
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    return (int)((rotation / rSpeed) + BASE_MOTOR_TIMEOUT_SEC + 0.5);
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
    
 #ifdef DEBUG
//    if(amount + 0.5 > 1.0)
//        std::cout << "decreased est print time by " << amount  << std::endl;
#endif    
   
}

/// Tells state machine that an interrupt has arrived from the motor controller,
/// and whether or not the expected motion completed successfully.
void PrintEngine::MotorCallback(unsigned char* status)
{
#ifdef DEBUG
//    std::cout << "in MotorCallback status = " << 
//                 ((int)*status) << 
//                 " at time = " <<
//                 GetMillis() << std::endl;
#endif    
    switch(*status)
    {
        case MC_ERROR:
            HandleError(MotorError, true);
            _pPrinterStateMachine->MotionCompleted(false);
            break;
            
        case MC_SUCCESS:
            _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            HandleError(UnknownMotorStatus, false, NULL, (int)*status);
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
/// (which may be translated into several low-level commands)
void PrintEngine::SendMotorCommand(int command)
{
#ifdef DEBUG    
// std::cout << "sending motor command: " << command << std::endl;
#endif  
    bool success = true;
    // layer thickness overrides are defined for the movement to the next layer
    int thickness = _layerSettings.GetInt(_printerStatus._currentLayer + 1, 
                                          LAYER_THICKNESS);
    
    switch(command)
    {
        case HOME_COMMAND:
            success = _pMotor->GoHome();
            if(!success)
                break;
            success = _pMotor->DisableMotors();
            break;
            
        case MOVE_TO_START_POSN_COMMAND: 
            success = _pMotor->EnableMotors();
            if(!success)
                break;
            success = _pMotor->GoToStartPosition();
            break;
            
        case SEPARATE_COMMAND:
            success = _pMotor->Separate(GetCurrentLayerType());
            break;
                        
        case APPROACH_COMMAND:
            success = _pMotor->Approach(GetCurrentLayerType(), thickness);
            break;
            
        case APPROACH_AFTER_JAM_COMMAND:
            success = _pMotor->Approach(GetCurrentLayerType(), thickness, true);
            break;
            
        case PAUSE_AND_INSPECT_COMMAND:
            success = _pMotor->PauseAndInspect(GetInspectRotation());
            break;
            
        case RESUME_FROM_INSPECT_COMMAND:
            success = _pMotor->ResumeFromInspect(GetInspectRotation());
            break;
            
        case JAM_RECOVERY_COMMAND:
            success = _pMotor->UnJam(GetCurrentLayerType());
            break;
            
        default:
            HandleError(UnknownMotorCommand, false, NULL, command);
            break;
    } 
    if(!success)    // the specific error was already logged
        HandleError(MotorError, true);
}

/// Cleans up from any print in progress
void PrintEngine::ClearCurrentPrint()
{
    PauseMovement();
    ClearPendingMovement();
    
    // log the temperature, for canceled prints or on fatal error
    char msg[50];
    sprintf(msg, LOG_TEMPERATURE, _temperature);
    LOGGER.LogMessage(LOG_INFO, msg); 
    
    // clear the number of layers
    SetNumLayers(0);
    // clear exposure timers
    ClearPreExposureDelayTimer();
    ClearExposureTimer();
    Exposing::ClearPendingExposureInfo();
    _printerStatus._estimatedSecondsRemaining = 0;
    // clear pause & inspect flags
    _pPrinterStateMachine->_atInspectionPosition = false;
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
    return PrintData::GetNumLayers(SETTINGS.GetString(PRINT_DATA_DIR)) >= 1;
}

/// See if we can start a print, and if so perform the necessary initialization
bool PrintEngine::TryStartPrint()
{
    ClearError();            
    _skipCalibration = false;
            
    // make sure we have valid data
    std::string printDataDir = SETTINGS.GetString(PRINT_DATA_DIR);
    if(!PrintData::Validate(printDataDir))
    {
       HandleError(NoValidPrintDataAvailable, true); 
       return false;
    }
    
    SetNumLayers(PrintData::GetNumLayers(printDataDir));
    
    // use per-layer settings, if file defining them exists
    _layerSettings.Load(printDataDir + PER_LAYER_SETTINGS_FILE);
    
    // make sure the temperature isn't too high to print
    if(IsPrinterTooHot())
        return false;
    
    // this would be a good point at which to validate print settings, 
    // if we knew the valid range for each

    // for the record:
    LogStatusAndSettings();
       
    ClearHomeUISubState();
    
#ifdef DEBUG
//    std::cout << "First layer time = " << GetLayerTime(First) << std::endl;
//    std::cout << "Burnin layer time = " << GetLayerTime(BurnIn) << std::endl;
//    std::cout << "Model layer time = " << GetLayerTime(Model) << std::endl;
#endif    
 
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

/// Gets the time required for separation from PDMS
double PrintEngine::GetSeparationTimeSec(LayerType type)
{
    double time, revs, sepRSpeed, z, sepZSpeed;
    
    switch(type)
    {
        case First:
            // start with extra delays in ms
            time = (SETTINGS.GetInt(FL_EXPOSURE_WAIT) +
                    SETTINGS.GetInt(FL_SEPARATION_WAIT)) / 1000.0;    
            // convert the angle of rotation in degrees/1000 to 
            // fractional revolutions
            revs = SETTINGS.GetInt(FL_ROTATION) / MILLIDEGREES_PER_REV;
            sepRSpeed = SETTINGS.GetInt(FL_SEPARATION_R_SPEED);
            // Z distances are in microns
            z = (double) SETTINGS.GetInt(FL_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(FL_SEPARATION_Z_SPEED);
            break;
            
        case BurnIn:
            time = (SETTINGS.GetInt(BI_EXPOSURE_WAIT) +
                    SETTINGS.GetInt(BI_SEPARATION_WAIT)) / 1000.0;       
            revs = SETTINGS.GetInt(BI_ROTATION) / MILLIDEGREES_PER_REV;
            sepRSpeed = SETTINGS.GetInt(BI_SEPARATION_R_SPEED);
            z = (double) SETTINGS.GetInt(BI_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(BI_SEPARATION_Z_SPEED);
            break;
            
        case Model:
            time = (SETTINGS.GetInt(ML_EXPOSURE_WAIT) +
                    SETTINGS.GetInt(ML_SEPARATION_WAIT)) / 1000.0;       
            revs = SETTINGS.GetInt(ML_ROTATION) / MILLIDEGREES_PER_REV;
            sepRSpeed = SETTINGS.GetInt(ML_SEPARATION_R_SPEED);
            z = (double) SETTINGS.GetInt(ML_Z_LIFT);
            sepZSpeed = SETTINGS.GetInt(ML_SEPARATION_Z_SPEED);
            break; 
    }
    
    // rotational speeds are in RPM
    revs *= 60;
    time += revs / sepRSpeed;
    
    // Z speeds are in microns/s
    time += z / sepZSpeed;
        
    return time;   
}

/// Gets the time required for approach back to PDMS
double PrintEngine::GetApproachTimeSec(LayerType type)
{
    double time, revs, approachRSpeed, z, approachZSpeed;
    
    switch(type)
    {
        case First:
            // convert the angle of rotation in degrees/1000 to 
            // fractional revolutions
            revs = SETTINGS.GetInt(FL_ROTATION) / MILLIDEGREES_PER_REV;
            approachRSpeed = SETTINGS.GetInt(FL_APPROACH_R_SPEED);
            // Z distances are in microns
            z = (double) SETTINGS.GetInt(FL_Z_LIFT);
            approachZSpeed =  SETTINGS.GetInt(FL_APPROACH_Z_SPEED);
            break;
            
        case BurnIn:
            revs = SETTINGS.GetInt(BI_ROTATION) / MILLIDEGREES_PER_REV;
            approachRSpeed = SETTINGS.GetInt(BI_APPROACH_R_SPEED);
            z = (double) SETTINGS.GetInt(BI_Z_LIFT);
            approachZSpeed =  SETTINGS.GetInt(BI_APPROACH_Z_SPEED);
            break;
            
        case Model:  
            revs = SETTINGS.GetInt(ML_ROTATION) / MILLIDEGREES_PER_REV;
            approachRSpeed = SETTINGS.GetInt(ML_APPROACH_R_SPEED);
            z = (double) SETTINGS.GetInt(ML_Z_LIFT);
            approachZSpeed =  SETTINGS.GetInt(ML_APPROACH_Z_SPEED);
            break; 
    }
    
    // rotational speeds are in RPM
    revs *= 60;
    time = revs / approachRSpeed;
    
    // Z speeds are in microns/s
    time += (z - SETTINGS.GetInt(LAYER_THICKNESS)) / approachZSpeed;
        
    return time;   
}

/// Gets the time (in seconds) required to print a layer based on the 
/// current settings for the type of layer.  Note: does not take into account
/// per-layer setting overrides that may change the actual print time.
double PrintEngine::GetLayerTimeSec(LayerType type)
{
    double time, revs, sepRSpeed, approachRSpeed, z, sepZSpeed, approachZSpeed;
    
    switch(type)
    {
        case First:
            // start with the exposure time, in seconds
            time = (double) SETTINGS.GetDouble(FIRST_EXPOSURE);
            // plus additional delay in ms
            time += SETTINGS.GetInt(FL_APPROACH_WAIT) / 1000.0;    
            break;
            
        case BurnIn:
            time = (double) SETTINGS.GetDouble(BURN_IN_EXPOSURE);
            time += SETTINGS.GetInt(BI_APPROACH_WAIT) / 1000.0;    
            break;
            
        case Model:
            time = (double) SETTINGS.GetDouble(MODEL_EXPOSURE);
            time += SETTINGS.GetInt(ML_APPROACH_WAIT) / 1000.0;    
            break; 
    }
    
    // add separation and approach times, and measured overhead 
    time += GetSeparationTimeSec(type) + GetApproachTimeSec(type) + 
            SETTINGS.GetDouble(LAYER_OVERHEAD);
    
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
    
    int layerThickness = _layerSettings.GetInt(_printerStatus._currentLayer,
                                               LAYER_THICKNESS);
    return SETTINGS.GetInt(MAX_Z_TRAVEL) > 
            (GetCurrentLayer() * layerThickness +  overlift +
            SETTINGS.GetInt(INSPECTION_HEIGHT));
}

/// Get the amount of rotation (in thousandths of a degree) to be used when
/// pausing for inspection.
int PrintEngine::GetInspectRotation()
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
void PrintEngine::ClearPendingMovement()
{
    if(!_pMotor->ClearPendingCommands())  
        HandleError(MotorError, true);
    
    ClearMotorTimeoutTimer();
    _remainingMotorTimeoutSec= 0.0;
}

