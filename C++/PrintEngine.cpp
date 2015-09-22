//  File:   PrintEngine.cpp
//  Defines the engine that controls the printing process
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

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

#include "PrinterStatusQueue.h"
#include "Timer.h"
#include "PrintFileStorage.h"

#define VIDEOFRAME__SEC         (1.0 / 60.0)
#define MILLIDEGREES_PER_REV    (360000.0)


// The only public constructor.  'haveHardware' can only be false in debug
// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware, Motor& motor,
        PrinterStatusQueue& printerStatusQueue, const Timer& exposureTimer,
        const Timer& temperatureTimer, const Timer& delayTimer,
        const Timer& motorTimeoutTimer) :
_haveHardware(haveHardware),
_homeUISubState(NoUISubState),
_invertDoorSwitch(false),
_temperature(-1.0),
_gotRotationInterrupt(false),
_alreadyOverheated(false),
_inspectionRequested(false),
_skipCalibration(false),
_remainingMotorTimeoutSec(0.0),
_demoModeRequested(false),
_printerStatusQueue(printerStatusQueue),
_exposureTimer(exposureTimer),
_temperatureTimer(temperatureTimer),
_delayTimer(delayTimer),
_motorTimeoutTimer(motorTimeoutTimer),
_motor(motor)
{
#ifndef DEBUG
    if (!haveHardware)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(HardwareNeeded));
        exit(-1);
    }
#endif  
    
    // construct the state machine and tell it this print engine owns it
    _pPrinterStateMachine = new PrinterStateMachine(this);  

    _invertDoorSwitch = (SETTINGS.GetInt(HARDWARE_REV) == 0);
    
    _pThermometer = new Thermometer(haveHardware);
    
    _pProjector = new Projector(PROJECTOR_SLAVE_ADDRESS, I2C0_PORT);

    // create a PrintData instance if previously loaded print data exists
    _pPrintData.reset(PrintData::CreateFromExistingData(
        SETTINGS.GetString(PRINT_DATA_DIR) + "/" + PRINT_DATA_NAME));
}

// Destructor
PrintEngine::~PrintEngine()
{
    delete _pPrinterStateMachine;
    delete _pThermometer;
    delete _pProjector;
   
}

// Starts the printer state machine.  Should not be called until event handler
// subscriptions are in place.
void PrintEngine::Begin()
{
    _pPrinterStateMachine->initiate();  
}

// Perform initialization that will be repeated whenever the state machine 
// enters the Initializing state (i.e. on startup and reset))
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
    
    if (!_motor.Initialize())  
        HandleError(MotorError, true);
}

// Send out the status of the print engine, including current temperature
// and status of any print in progress 
void PrintEngine::SendStatus(PrintEngineState state, StateChange change, 
                             UISubState substate)
{
    _printerStatus._state = state;
    _printerStatus._UISubState = substate;
    _printerStatus._change = change;
    _printerStatus._temperature = _temperature;

    _printerStatusQueue.Push(_printerStatus);
}

// Return the most recently set UI sub-state
UISubState PrintEngine::GetUISubState()
{
    return _printerStatus._UISubState;
}

// Translate the event handler events into state machine events
void PrintEngine::Callback(EventType eventType, const EventData& data)
{
    double exposureTimeLeft;
    
    switch(eventType)
    {
        case MotorInterrupt:
            MotorCallback(data.Get<unsigned char>());
            break;
            
        case ButtonInterrupt:
            ButtonCallback(data.Get<unsigned char>());
            break;

        case DoorInterrupt:
            DoorCallback(data.Get<char>());
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
                                                    data.Get<unsigned char>());
            _pPrinterStateMachine->MotionCompleted(false);
            break;
           
        case TemperatureTimer:
            // don't read thermometer while exposing, as it may reduce the
            // accuracy of the exposure time
            exposureTimeLeft = GetRemainingExposureTimeSec();
            if (exposureTimeLeft > 0.0)
                StartTemperatureTimer(exposureTimeLeft + 0.1);
            else
            {
                // read and record temperature
                _temperature = _pThermometer->GetTemperature();
   
                if (!_alreadyOverheated)
                    IsPrinterTooHot();
                
                // keep reading temperature even if we're already overheated
                StartTemperatureTimer(TEMPERATURE_MEASUREMENT_INTERVAL_SEC);
            }
            break;
           
        case USBDriveConnected:
            USBDriveConnectedCallback(data.Get<std::string>());
            break;

        case USBDriveDisconnected:
            USBDriveDisconnectedCallback();
            break;

        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), 
                                                                    eventType);
            break;
    }
}

// Handle commands that have already been interpreted
void PrintEngine::Handle(Command command)
{  
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
            remove(TEMP_SETTINGS_FILE);
            if (!result)
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
            
        case Dismiss:
            _pPrinterStateMachine->process_event(EvDismiss());
            break;
    
    // the following commands may be used by automated test applications to
    // simulate front panel button actions
        case Button1:
            _pPrinterStateMachine->process_event(EvLeftButton());
            break;
            
        case Button2:
            _pPrinterStateMachine->process_event(EvRightButton());
            break;
 
        case Button1Hold:
            _pPrinterStateMachine->process_event(EvLeftButtonHold());
            break;

        case Button2Hold:
            _pPrinterStateMachine->process_event(EvRightButtonHold());
            break;
            
        case Buttons1and2:
            _pPrinterStateMachine->process_event(EvLeftAndRightButton());
            break;

        case Buttons1and2Hold:
            _pPrinterStateMachine->process_event(EvLeftAndRightButtonHold());
            break;

        case Exit:
            // EventHandler handles exit
            break;
            
        default:
            HandleError(UnknownCommandInput, false, NULL, command); 
            break;
    }
}

// Converts button events from UI board into state machine events
void PrintEngine::ButtonCallback(unsigned char status)
{ 
    unsigned char maskedStatus = 0xF & status;

    if (maskedStatus == 0)
    {
        // ignore any non-button events for now
        return;
    }
    
    // check for error status, in unmasked value
    if (status == ERROR_STATUS)
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
                                                      static_cast<int>(status));
            break;
    }        
}

// Start the timer used for various delays.
void PrintEngine::StartDelayTimer(double seconds)
{
    try
    {
        _delayTimer.Start(seconds);
    }
    catch (const std::runtime_error& e)
    {
        HandleError(PreExposureDelayTimer, true);  
    }
}

// Clears the timer used for various delays 
void PrintEngine::ClearDelayTimer()
{
    try
    {
        _delayTimer.Clear();
    }
    catch (const std::runtime_error& e)
    {
        HandleError(PreExposureDelayTimer, true);  
    }
}

// Get the pre exposure delay time for the current layer
double PrintEngine::GetPreExposureDelayTimeSec()
{    
    // settings are in milliseconds
    return _cls.ApproachWaitMS / 1000.0;
}

// Start the timer whose expiration signals the end of exposure for a layer
void PrintEngine::StartExposureTimer(double seconds)
{
    try
    {
        _exposureTimer.Start(seconds);
    }
    catch (const std::runtime_error& e)
    {
        HandleError(ExposureTimer, true);  
    }
}

// Clears the timer whose expiration signals the end of exposure for a layer
void PrintEngine::ClearExposureTimer()
{
    try
    {
        _exposureTimer.Clear();
    }
    catch (const std::runtime_error& e)
    {
        HandleError(ExposureTimer, true);  
    }
}

// Get the exposure time for the current layer
double PrintEngine::GetExposureTimeSec()
{
    double expTime = _cls.ExposureSec;

    // actual exposure time includes an extra video frame, 
    // so reduce the requested time accordingly
    if (expTime > VIDEOFRAME__SEC)
        expTime -= VIDEOFRAME__SEC;
    
    return expTime;
}

// Returns true if and only if the current layer is the first one
bool PrintEngine::IsFirstLayer()
{
    return _printerStatus._currentLayer == 1;
}

// Returns true if and only if the current layer is a burn-in layer
bool PrintEngine::IsBurnInLayer()
{
    int numBurnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
    return (numBurnInLayers > 0 && 
            _printerStatus._currentLayer > 1 &&
            _printerStatus._currentLayer <= 1 + numBurnInLayers);
}

// Start the timer whose expiration indicates that the motor controller hasn't 
// signaled its command completion in the expected time
void PrintEngine::StartMotorTimeoutTimer(int seconds)
{
    try
    {
        _motorTimeoutTimer.Start(seconds);
    }
    catch (const std::runtime_error& e)
    {
        HandleError(MotorTimeoutTimer, true);  
    }
}

// Start (or restart) the timer whose expiration signals that it's time to 
// measure the temperature
void PrintEngine::StartTemperatureTimer(double seconds)
{
    try
    {
        _temperatureTimer.Start(seconds);
    }
    catch (const std::runtime_error& e)
    {
        HandleError(TemperatureTimerError, true);  
    }
}

// Clears the timer whose expiration indicates that the motor controller hasn't 
// signaled its command completion in the expected time
void PrintEngine::ClearMotorTimeoutTimer()
{
    try
    {
        _motorTimeoutTimer.Clear();
    }
    catch (const std::runtime_error& e)
    {
        HandleError(MotorTimeoutTimer, true);  
    }
}

// Set or clear the number of layers in the current print.  
// Also resets the current layer number.
void PrintEngine::SetNumLayers(int numLayers)
{
    _printerStatus._numLayers = numLayers;
    // the number of layers should only be set before starting a print,
    // or when clearing it at the end or canceling of a print
    _printerStatus._currentLayer = 0;
}

// Increment the current layer number and attempt to load its image.  Returns
// true only if that succeeds. Logs temperature on the quartiles.
bool PrintEngine::NextLayer()
{
    bool retVal = false;
    SDL_Surface* image;
    
    ++_printerStatus._currentLayer;  
    
    if (!_pPrintData || 
        !(image = _pPrintData->GetImageForLayer(_printerStatus._currentLayer)))
    {
        // if no image available, there's no point in proceeding
        HandleError(NoImageForLayer, true, NULL,
                    _printerStatus._currentLayer);
        ClearCurrentPrint(); 
    }
    else
    {
        // see if we should scale the image
        double scale = SETTINGS.GetDouble(IMAGE_SCALE_FACTOR);
        if (scale != 1.0)
            _pProjector->ScaleImage(image, scale);
        
        // update projector with image
        _pProjector->SetImage(image);
        
        // log temperature at start, end, and quartile points
        int layer = _printerStatus._currentLayer;
        int total = _printerStatus._numLayers;
        if (layer == 1 || 
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

// Returns true or false depending on whether or not the current print
// has any more layers to be printed.
bool PrintEngine::NoMoreLayers()
{
    if (_printerStatus._currentLayer >= _printerStatus._numLayers)
    {
        // clear the print-in-progress status
        SetEstimatedPrintTime(false);
        return true;
    }
    else
        return false;
}

// Sets or clears the estimated print time
void PrintEngine::SetEstimatedPrintTime(bool set)
{
    if (set)
    {
        int layersLeft = _printerStatus._numLayers - 
                        (_printerStatus._currentLayer - 1);
        
        double burnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
        double burnInTime = GetLayerTimeSec(BurnIn);
        double modelTime = GetLayerTimeSec(Model);
        double layerTimes = 0.0;
        
        // remaining time depends first on what kind of layer we're in
        if (IsFirstLayer())
        {
            layerTimes = GetLayerTimeSec(First) +
                         burnInLayers * burnInTime + 
                         (_printerStatus._numLayers - (burnInLayers + 1)) * 
                                                                  modelTime;
        } 
        else if (IsBurnInLayer())
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
}

// Update the estimated time remaining for the print
void PrintEngine::DecreaseEstimatedPrintTime(double amount)
{
    _printerStatus._estimatedSecondsRemaining -= (int)(amount + 0.5);
}

// Tells state machine that an interrupt has arrived from the motor controller,
// and whether or not the expected motion completed successfully.
void PrintEngine::MotorCallback(unsigned char status)
{
    // clear the pending timeout
    ClearMotorTimeoutTimer();
    
    switch(status)
    {        
        case MC_STATUS_SUCCESS:
            _pPrinterStateMachine->MotionCompleted(true);
            break;
            
        default:
            // any motor error is fatal
            HandleError(MotorControllerError, true, NULL, 
                                                    static_cast<int>(status));
            _pPrinterStateMachine->MotionCompleted(false);
            break;
    }    
}

// Tells the state machine to handle door sensor events
void PrintEngine::DoorCallback(char data)
{      
    if (data == (_invertDoorSwitch ? '1' : '0'))
        _pPrinterStateMachine->process_event(EvDoorClosed());
    else
        _pPrinterStateMachine->process_event(EvDoorOpened());
}
     
// Handles errors with message and optional parameters
void PrintEngine::HandleError(ErrorCode code, bool fatal, 
                              const char* str, int value)
{
    char* msg;
    int origErrno = errno;
    // log the error
    const char* baseMsg = ERR_MSG(code);
    if (str != NULL)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                          str);
    else if (value != INT_MAX)
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, baseMsg, 
                                                                        value);
    else
        msg = LOGGER.LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, 
                                                                       baseMsg);
    
    // before setting any error codes into status:
    LogStatusAndSettings();
    
    // Report fatal errors and put the state machine in the Error state 
    if (fatal) 
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

// log firmware version, current print status, & current settings
void PrintEngine::LogStatusAndSettings()
{
    LOGGER.LogMessage(LOG_INFO, (std::string(FW_VERSION_MSG) + 
                                 GetFirmwareVersion()).c_str());
    LOGGER.LogMessage(LOG_INFO, _printerStatus.ToString().c_str());
    LOGGER.LogMessage(LOG_INFO, SETTINGS.GetAllSettingsAsJSONString().c_str());    
}

// Clear the last error from printer status to be reported next
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

// Send a high-level command to the motor controller 
// (which may be translated into several low-level commands),
// and set the timeout timer.
void PrintEngine::SendMotorCommand(int command)
{
    bool success = true;
        
    switch(command)
    {
        case HOME_COMMAND:
            success = _motor.GoHome();
            StartMotorTimeoutTimer(GetHomingTimeoutSec());
            break;
            
        case MOVE_TO_START_POSN_COMMAND: 
            success = _motor.GoToStartPosition();
            // for tracking where we are, to enable lifting for inspection
            _currentZPosition = 0;
            StartMotorTimeoutTimer(GetStartPositionTimeoutSec());
            break;
            
        case SEPARATE_COMMAND:
            success = _motor.Separate(_cls);
            StartMotorTimeoutTimer(GetSeparationTimeoutSec());
            break;
                        
        case APPROACH_COMMAND:
            success = _motor.Approach(_cls);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec());
            break;
            
        case APPROACH_AFTER_JAM_COMMAND:
            success = _motor.Approach(_cls, true);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec() +
                                   GetUnjammingTimeoutSec());
            break;
            
        case PRESS_COMMAND:
            success = _motor.Press(_cls);
            StartMotorTimeoutTimer(GetPressTimeoutSec());
            break;
            
         case UNPRESS_COMMAND:
            success = _motor.Unpress(_cls);
            StartMotorTimeoutTimer(GetUnpressTimeoutSec());
            break;
 
        case PAUSE_AND_INSPECT_COMMAND:
            success = _motor.PauseAndInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(true));
            break;
            
        case RESUME_FROM_INSPECT_COMMAND:
            success = _motor.ResumeFromInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(false));
            break;
            
        case JAM_RECOVERY_COMMAND:
            success = _motor.UnJam(_cls);
            StartMotorTimeoutTimer(GetUnjammingTimeoutSec());
            break;
            
        default:
            HandleError(UnknownMotorCommand, false, NULL, command);
            break;
    } 
    if (!success)    // the specific error was already logged
        HandleError(MotorError, true);
}

// Cleans up from any print in progress.  If withInterrupt, and interrupt will 
// be requested when clearing any pending movement, in case a movement is 
// currently in progress.
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

// Indicate that no print job is in progress
void PrintEngine::ClearJobID()
{
    _printerStatus._jobID = "";
    
    SETTINGS.Set(JOB_ID_SETTING, std::string(""));
    SETTINGS.Save(); 
    
    // get a new unique ID for the next local job (if any)
    GetUUID(_printerStatus._localJobUniqueID); 
}

// Find the remaining exposure time 
double PrintEngine::GetRemainingExposureTimeSec()
{
    try
    {
        return _exposureTimer.GetRemainingTimeSeconds();
    }
    catch (const std::runtime_error& e)
    {
        HandleError(RemainingExposure, true);  
    }
}

// Determines if the door is open or not
bool PrintEngine::DoorIsOpen()
{
    if (!_haveHardware)
        return false;
    
    char GPIOInputValue[64], value;
    
    sprintf(GPIOInputValue, GPIO_VALUE, DOOR_SENSOR_PIN);
    
    // Open the file descriptor for the door switch GPIO
    int fd = open(GPIOInputValue, O_RDONLY);
    if (fd < 0)
    {
        HandleError(GpioInput, true, NULL, DOOR_SENSOR_PIN);
        throw std::runtime_error(ErrorMessage::Format(GpioInput, 
                                                      DOOR_SENSOR_PIN, errno));
    }  
    
    read(fd, &value, 1);

    close(fd);

	return (value == (_invertDoorSwitch ? '0' : '1'));
}

// Wraps Projector's ShowImage method and handles errors
void PrintEngine::ShowImage()
{
    if (!_pProjector->ShowImage())
    {
        HandleError(CantShowImage, true, NULL, _printerStatus._currentLayer);
        ClearCurrentPrint();  
    }  
}
 
// Wraps Projector's ShowBlack method and handles errors
void PrintEngine::ShowBlack()
{
    if (!_pProjector->ShowBlack())
    {
        HandleError(CantShowBlack, true);
        ClearCurrentPrint();  
    }
}

// Returns true if and only if there is at least one layer image present 
// (though it/they may still not be valid for printing)
bool PrintEngine::HasAtLeastOneLayer()
{
    return _pPrintData && _pPrintData->GetLayerCount() >= 1;
}

// See if we can start a print, and if so perform the necessary initialization
bool PrintEngine::TryStartPrint()
{
    ClearError();            
    _skipCalibration = false;
            
    // make sure we have valid data
    if (!_pPrintData || !_pPrintData->Validate())
    {
       HandleError(NoValidPrintDataAvailable, true); 
       return false;
    }
    
    SetNumLayers(_pPrintData->GetLayerCount());
   
    // clear per-layer settings in case they exist from a previous print
    _perLayer.Clear();
    
    // use per-layer settings, if print data contains them
    std::string perLayerSettings;
    if (_pPrintData->GetFileContents(PER_LAYER_SETTINGS_FILE, perLayerSettings))
        _perLayer.Load(perLayerSettings);
    
    // make sure the temperature isn't too high to print
    if (IsPrinterTooHot())
        return false;
    
    // this would be a good point at which to validate print settings, 
    // if we knew the valid range for each

    // for the record:
    LogStatusAndSettings();
       
    ClearHomeUISubState();
     
    return true;
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

// Begin process of loading print data from USB storage.
void PrintEngine::USBDriveConnectedCallback(const std::string& deviceNode)
{
    // ensure valid state
    if (!(_printerStatus._state == HomeState &&
            _printerStatus._UISubState != LoadingPrintData &&
            _printerStatus._UISubState != DownloadingPrintData))
        return;

    if (!Mount(deviceNode, USB_DRIVE_MOUNT_POINT, "vfat")) 
    {
        HandleError(UsbDriveMount, false, deviceNode.c_str());
        ShowHomeScreenFor(USBDriveError); 
        return;
    }

    std::ostringstream path;
    path << USB_DRIVE_MOUNT_POINT << "/" << 
                                        SETTINGS.GetString(USB_DRIVE_DATA_DIR);

    PrintFileStorage storage(path.str());

    if (!storage.HasOneFile())
    {
        ShowHomeScreenFor(USBDriveError); 
        return;
    }

    _printerStatus._usbDriveFileName = storage.GetFileName();
    ShowHomeScreenFor(USBDriveFileFound); 
}

// Unmount the USB drive.
// Dismiss the screen displayed as a result of USB drive connection and return
// to the appropriate UISubState if still in a UISubstate triggered by a USB
// drive event.
void PrintEngine::USBDriveDisconnectedCallback()
{
    umount(USB_DRIVE_MOUNT_POINT);

    if (_printerStatus._state == HomeState && (
            _printerStatus._UISubState == USBDriveFileFound ||
            _printerStatus._UISubState == USBDriveError))
    {
        ShowHomeScreenFor(HasAtLeastOneLayer() ? HavePrintData : NoPrintData);
    }
}

// Load the print file from the attached USB drive.
void PrintEngine::LoadPrintFileFromUSBDrive()
{
    ShowHomeScreenFor(LoadingPrintData);

    // copy the file from the USB drive to the download directory
    // print data processing moves or deletes the found file and we don't want
    // to move or delete the user's file from her or his usb drive

    std::ostringstream path;
    path << USB_DRIVE_MOUNT_POINT << "/" << 
                                        SETTINGS.GetString(USB_DRIVE_DATA_DIR);

    PrintFileStorage storage(path.str());

    Copy(storage.GetFilePath(), SETTINGS.GetString(DOWNLOAD_DIR));

    ProcessData();
}

// Prepare downloaded print data for printing.
// Looks for print file in specified directory.
void PrintEngine::ProcessData()
{
    PrintFileStorage storage(SETTINGS.GetString(DOWNLOAD_DIR));
    
    // If any processing step fails, clear downloading screen, report an error,
    // and return to prevent any further processing

    // construct an instance of a PrintData object using a file from the 
    // download directory
    boost::scoped_ptr<PrintData> pNewPrintData(PrintData::CreateFromNewData(
            storage, SETTINGS.GetString(STAGING_DIR),
            PRINT_DATA_NAME));

    if (!pNewPrintData)
    {
        // no incoming print file found
        HandleProcessDataFailed(CantStageIncomingPrintData, "");
        return;
    }
    
    if (!pNewPrintData->Validate())
    {
        // invalid print data
        HandleProcessDataFailed(InvalidPrintData, storage.GetFileName());
        return;
    }
    
    // first restore all print settings to their defaults, in case the new
    // settings don't include all possible settings (e.g. because the print data
    // file was created before some newer settings were defined)
    if (!SETTINGS.RestoreAllPrintSettings())
        // error logged in Settings
        return;

    bool settingsLoaded = false;

    // determine if a temp settings file exists, containing settings for 
    // incoming data
    if (std::ifstream(TEMP_SETTINGS_FILE))
        // use settings from temp file
        settingsLoaded = SETTINGS.SetFromFile(TEMP_SETTINGS_FILE);
    else
    {
        // use settings from file contained in print data
        std::string settings;
        if (pNewPrintData->GetFileContents(EMBEDDED_PRINT_SETTINGS_FILE, 
                                                                    settings))
            settingsLoaded = SETTINGS.SetFromJSONString(settings);
    }

    // remove the temp settings file
    // the contained settings apply only to the incoming data    
    remove(TEMP_SETTINGS_FILE);
    
    if (!settingsLoaded)
    {
        HandleProcessDataFailed(CantLoadSettingsForPrintData, 
                                                        storage.GetFileName());
        return;
    }

    // if old data exists, remove it now that this method has validated the data
    // and loaded the settings successfully
    // if the remove operation fails, don't consider it an error since someone
    // or something could have removed the underlying data from the actual 
    // storage device
    if (_pPrintData)
    {
        _pPrintData->Remove();
    }

    // move the new print data from the staging directory to the print data 
    // directory
    if (!pNewPrintData->Move(SETTINGS.GetString(PRINT_DATA_DIR)))
    {
        // if moving the new print data into place fails, the printer does not
        // have any print data present
        // clear settings set by the attempted load
        SETTINGS.Set(JOB_ID_SETTING, "");
        SETTINGS.Set(JOB_NAME_SETTING, "");

        // clear state that this function otherwise overwrites if the move
        // operation succeeds
        SETTINGS.Set(PRINT_FILE_SETTING, "");
        _printerStatus._jobID = "";
        
        SETTINGS.Save();

        HandleProcessDataFailed(CantMovePrintData, storage.GetFileName());
        return;
    }
    
    // Update PrintEngine's reference so it points to the newly processed print 
    // data.  After the swap, the smart pointer pNewPrintData will delete the 
    // "old" print data instance when it goes out of scope and the _pPrintData 
    // member variable will point to the "new" print data instance.
    _pPrintData.swap(pNewPrintData);
    
    // record the name of the last file downloaded
    SETTINGS.Set(PRINT_FILE_SETTING, storage.GetFileName());
    SETTINGS.Save();
   
    // update the printer status with the job id
    _printerStatus._jobID = SETTINGS.GetString(JOB_ID_SETTING);
    
    ShowHomeScreenFor(LoadedPrintData);
}

// Convenience method handles the error and sends status update with
// UISubState needed to show that processing data failed on the front panel
// (unless we're already showing an error)
// Also ensures removal of temp settings file
void PrintEngine::HandleProcessDataFailed(ErrorCode errorCode, 
                                          const std::string& jobName)
{
    remove(TEMP_SETTINGS_FILE);
    HandleError(errorCode, false, jobName.c_str());
    _homeUISubState = PrintDataLoadFailed;
    // don't send new status if we're already showing a fatal error
    if (_printerStatus._state != ErrorState)
        SendStatus(_printerStatus._state, NoChange, PrintDataLoadFailed);
}

// Delete any existing printable data.
void PrintEngine::ClearPrintData()
{
    if (_pPrintData && _pPrintData->Remove())
    {
        ClearHomeUISubState();
        // also clear job name, ID, and last print file
        std::string empty = "";
        SETTINGS.Set(JOB_NAME_SETTING, empty);
        SETTINGS.Set(PRINT_FILE_SETTING, empty);
        ClearJobID();   // also save settings changes
        // dispose of PrintData instance
        _pPrintData.reset(NULL);
    }
    else
        HandleError(CantRemovePrintData);        
}

// Gets the time (in seconds) required to print a layer based on the 
// current settings for the type of layer.  Note: does not take into account
// per-layer setting overrides that may change the actual print time.
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
            if (press != 0)
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
            if (press != 0)
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
            if (press != 0)
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

// Checks to see if the printer is too hot to function
bool PrintEngine::IsPrinterTooHot()
{
    _alreadyOverheated = false;
    if (_temperature > SETTINGS.GetDouble(MAX_TEMPERATURE))
    {
        char val[20];
        sprintf(val, "%g", _temperature);
        HandleError(OverHeated, true, val);
        _alreadyOverheated =  true;
    }
    
    return _alreadyOverheated;
}

// Check to see if we got the expected interrupt from the rotation sensor.
bool PrintEngine::GotRotationInterrupt()
{ 
    if (SETTINGS.GetInt(DETECT_JAMS) == 0 || // jam detection disabled or
       SETTINGS.GetInt(HARDWARE_REV) == 0)   // old hardware lacking this sensor
        return true; 
    
    return _gotRotationInterrupt;
}

// Record whether or not a pause & inspect has been requested, 
// and set UI sub-state if it has been requested.
void PrintEngine::SetInspectionRequested(bool requested) 
{
    _inspectionRequested = requested; 
    if (requested)
        SendStatus(_printerStatus._state, NoChange, AboutToPause);
}

// Pause any movement in progress immediately (not a pause for inspection.)
void PrintEngine::PauseMovement()
{
    if (!_motor.Pause())   
        HandleError(MotorError, true);
    
    _remainingMotorTimeoutSec = _motorTimeoutTimer.GetRemainingTimeSeconds();
    
    ClearMotorTimeoutTimer();
}

// Resume any paused movement in progress (not a resume from inspection.)
void PrintEngine::ResumeMovement()
{
    if (!_motor.Resume())
    {
        HandleError(MotorError, true);
        return;
    }
    
    if (_remainingMotorTimeoutSec > 0.0)
    {
        // resume the motor timeout timer too, for at least a second
        StartMotorTimeoutTimer((int)std::max(_remainingMotorTimeoutSec, 1.0));
        _remainingMotorTimeoutSec= 0.0;
    }
}

// Abandon any movements still pending after a pause.
void PrintEngine::ClearPendingMovement(bool withInterrupt)
{
    if (!_motor.ClearPendingCommands(withInterrupt))  
        HandleError(MotorError, true);
    
    ClearMotorTimeoutTimer();
    _remainingMotorTimeoutSec= 0.0;
    
    if (withInterrupt)  // set timeout for awaiting completion of of the clear
        StartMotorTimeoutTimer((int)SETTINGS.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
}

// Get the amount of tray deflection (if any) wanted after approach.
int PrintEngine::GetTrayDeflection()
{
    return _cls.PressMicrons;
}

// Get the length of time to pause after tray deflection.
double PrintEngine::GetTrayDeflectionPauseTimeSec()
{
    // convert from milliseconds
    return _cls.PressWaitMS / 1000.0;
}

// Pad the raw expected time for a movement to get a reasonable timeout period.
int  PrintEngine::PadTimeout(double rawTime)
{
    return (int) (rawTime * SETTINGS.GetDouble(MOTOR_TIMEOUT_FACTOR) + 
                            SETTINGS.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
}

// Returns the timeout (in seconds) to allow for getting to the home position
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

// Returns the timeout (in seconds) to allow for getting to the start position
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

// Returns the timeout (in seconds) to allow for moving to or from the pause 
// and inspect position.
int PrintEngine::GetPauseAndInspectTimeoutSec(bool toInspect)
{   
    double zSpeed, rSpeed;
    
    if (toInspect)
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

// Returns the timeout (in seconds) to allow for attempting to recover from a
// jam, which depends on the type of layer.
int PrintEngine::GetUnjammingTimeoutSec()
{   
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;

    // assume we may take twice as long as normal to get to the home position,
    // and then we also need to rotate back to the separation position
    revs *= 3.0; 
    // convert to revolutions per second
    return PadTimeout((revs / _cls.SeparationRPM) * 60.0);
}

// Get the time required for the tray deflection movement.
int PrintEngine::GetPressTimeoutSec()
{
    return PadTimeout(_cls.PressMicrons / (double) _cls.PressMicronsPerSec);
}

// Get the time required for moving back from tray deflection.
int PrintEngine::GetUnpressTimeoutSec()
{
    return PadTimeout(_cls.PressMicrons / (double) _cls.UnpressMicronsPerSec);
}

// Gets the time required for separation from PDMS.  Assumes infinite jerk.
int PrintEngine::GetSeparationTimeoutSec()
{    
    // rotational speeds are in RPM
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;
    double time = (revs / _cls.SeparationRPM) * 60.0;
    
    // Z speeds are in microns/s
    time += _cls.ZLiftMicrons / (double) _cls.SeparationMicronsPerSec;
        
    return PadTimeout(time);   
}

// Gets the time required for approach back to PDMS
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

// Read all of the settings applicable to the current layer into a struct
// for reuse without having to look them up again.  This method should be 
// called once per layer, before the layer number has been incremented 
// for exposure.
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
    if (n == 1)
        type = First;
    else
    {
        int numBurnInLayers = SETTINGS.GetInt(BURN_IN_LAYERS);
        if (numBurnInLayers > 0 && n <= 1 + numBurnInLayers)
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
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, 
                                                        FL_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, FL_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, FL_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, FL_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, FL_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, 
                                                        FL_APPROACH_Z_SPEED);
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
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, 
                                                        BI_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, BI_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, BI_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, BI_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, BI_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, 
                                                        BI_APPROACH_Z_SPEED);
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
            _cls.SeparationMicronsPerSec = _perLayer.GetInt(p, 
                                                        ML_SEPARATION_Z_SPEED);
            _cls.ZLiftMicrons = _perLayer.GetInt(p, ML_Z_LIFT);
            _cls.ApproachRotJerk = _perLayer.GetInt(p, ML_APPROACH_R_JERK);
            _cls.ApproachRPM = _perLayer.GetInt(p, ML_APPROACH_R_SPEED);
            _cls.ApproachZJerk = _perLayer.GetInt(p, ML_APPROACH_Z_JERK);
            _cls.ApproachMicronsPerSec = _perLayer.GetInt(p, 
                                                        ML_APPROACH_Z_SPEED);
            break;
    }
    
    // likewise any layer thickness overrides come from the next layer
    _cls.LayerThicknessMicrons = _perLayer.GetInt(p, LAYER_THICKNESS);
    
    // to avoid changes while pause & inspect is already in progress:
    _cls.InspectionHeightMicrons = SETTINGS.GetInt(INSPECTION_HEIGHT);
    // see if there's enough headroom to lift the model for inspection.
    _cls.CanInspect = (_cls.InspectionHeightMicrons != 0) && 
                      (SETTINGS.GetInt(MAX_Z_TRAVEL) > (_currentZPosition +  
                                                        _cls.ZLiftMicrons +
                                                _cls.InspectionHeightMicrons));
}

// Indicate whether the last print is regarded as successful or failed.
void PrintEngine::SetPrintFeedback(PrintRating rating)
{
    _printerStatus._printRating = rating;
}

// Determines if the front panel's right button was depressed the first time 
// this method was called (at startup), indicating that the user wants the 
// printer to enter demo mode.
bool PrintEngine::DemoModeRequested()
{
    if (!_haveHardware || SETTINGS.GetInt(HARDWARE_REV) == 0)
        return false;
    
    static bool firstTime = true;
    
    if (firstTime)
    {
        firstTime = false;  // only do this once
        
        // read the GPIO connected to the front panel button        
        // setup GPIO as input pin
        char GPIOInputString[4], GPIOInputValue[64], GPIODirection[64], 
             setValue[10], value;
        FILE *inputHandle = NULL;

        // setup input
        sprintf(GPIOInputString, "%d", BUTTON2_DIRECT);
        sprintf(GPIOInputValue, GPIO_VALUE, BUTTON2_DIRECT);
        sprintf(GPIODirection, GPIO_DIRECTION, BUTTON2_DIRECT);

        // export & configure the pin
        if ((inputHandle = fopen(GPIO_EXPORT, "ab")) == NULL)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioExport), 
                                                                BUTTON2_DIRECT);
            return false;
        }
        strcpy(setValue, GPIOInputString);
        fwrite(&setValue, sizeof(char), 2, inputHandle);
        fclose(inputHandle);

        // Set direction of the pin to an input
        if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioDirection), 
                                                                BUTTON2_DIRECT);
            return false;
        }
        strcpy(setValue,"in");
        fwrite(&setValue, sizeof(char), 2, inputHandle);
        fclose(inputHandle);        

        sprintf(GPIOInputValue, GPIO_VALUE, BUTTON2_DIRECT);

        // Open the file descriptor for the front panel button's GPIO
        int fd = open(GPIOInputValue, O_RDONLY);
        if (fd < 0)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioInput), BUTTON2_DIRECT);
            return false;
        }  

        read(fd, &value, 1);
        _demoModeRequested = (value == '0');
        close(fd);
        
        // Unexport the pin
        if ((inputHandle = fopen(GPIO_UNEXPORT, "ab")) == NULL) 
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioUnexport));
        }
        strcpy(setValue, GPIOInputString);
        fwrite(&setValue, sizeof(char), 2, inputHandle);
        fclose(inputHandle);   
    }
    
    return _demoModeRequested;
}

// Put the printer into demo mode, with the projector full on.
bool PrintEngine::SetDemoMode()
{
    Initialize();
        
    // go to home position without rotating the tray to cover the projector
    _motor.GoHome(true, true);  
    // (and leave the motors enabled to hold their positions)
    
    _pProjector->ShowWhite();
}
