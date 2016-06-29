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
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <Hardware.h>
#include <PrintEngine.h>
#include <PrinterStateMachine.h>
#include <Logger.h>
#include <Filenames.h>
#include <PrintData.h>
#include <utils.h>
#include <Shared.h>
#include <MessageStrings.h>
#include <MotorController.h>
#include <Projector.h>

#include "PrinterStatusQueue.h"
#include "Timer.h"
#include "PrintFileStorage.h"

constexpr double VIDEOFRAME__SEC        = 1.0 / 60.0;
constexpr double MILLIDEGREES_PER_REV   = 360000.0;


// The only public constructor.  'haveHardware' can only be false in debug
// builds, for test purposes only.
PrintEngine::PrintEngine(bool haveHardware, Motor& motor, Projector& projector,
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
_projector(projector),
_motor(motor),
_bgndThread(0),
_settings(PrinterSettings::Instance())
{
#ifndef DEBUG
    if (!haveHardware)
    {
        Logger::LogError(LOG_ERR, errno, HardwareNeeded);
        exit(-1);
    }
#endif  
    
    // construct the state machine and tell it this print engine owns it
    _pPrinterStateMachine = new PrinterStateMachine(this);  

    _invertDoorSwitch = (_settings.GetInt(HARDWARE_REV) == 0);
    
    _pThermometer = new Thermometer(haveHardware);
    
    // create a PrintData instance if previously loaded print data exists
    _pPrintData.reset(PrintData::CreateFromExistingData(
        _settings.GetString(PRINT_DATA_DIR) + "/" + PRINT_DATA_NAME));
}

// Destructor
PrintEngine::~PrintEngine()
{
    if(_bgndThread != 0)
        pthread_cancel(_bgndThread);

    delete _pPrinterStateMachine;
    delete _pThermometer;
}

// Starts the printer state machine.  Should not be called until event handler
// subscriptions are in place.
void PrintEngine::Begin()
{
    // set video or pattern mode first (in case we're going into demo mode)
    SetPrintMode();  
    _printerStatus._canUpgradeProjector = _projector.CanUpgrade();
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
            Logger::LogError(LOG_WARNING, errno, UnexpectedEvent, eventType);
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
            _projector.ShowBlack();
            _pPrinterStateMachine->process_event(EvReset());
            break;
            
        case Test:           
            // show a test pattern, regardless of whatever else we're doing,
            // since this command is for test & setup only
            try
            {
                Magick::Image image(GetFilePath(TEST_PATTERN_FILE));
                _projector.SetImage(image);
                _projector.ShowCurrentImage();
            }
            catch (const std::exception& e)
            {
                Logger::LogError(LOG_WARNING, errno, LoadImageError,
                                GetFilePath(TEST_PATTERN_FILE));
            }
            break;
            
        case CalImage:           
            // show a calibration imagen, regardless of what we're doing,
            // since this command is for test & setup only
            try
            {
                Magick::Image image(GetFilePath(CAL_IMAGE_FILE));
                _projector.SetImage(image);
                _projector.ShowCurrentImage();
            }
            catch (const std::exception& e)
            {
                Logger::LogError(LOG_WARNING, errno, LoadImageError,
                                GetFilePath(CAL_IMAGE_FILE));
            }
            break;
        
        case RefreshSettings:
            // reload the settings file
            _settings.Refresh();
            LogStatusAndSettings(); //for the record
            break;
            
        case ApplySettings:
            // load the settings for the printer or a print
            result = _settings.SetFromFile(TEMP_SETTINGS_FILE);
            remove(TEMP_SETTINGS_FILE);
            if (!result)
                HandleError(CantLoadSettingsFile, true, TEMP_SETTINGS_FILE);
            break;
            
        case ShowPrintDataDownloading:
            ShowScreenFor(DownloadingPrintData); 
            break;
            
        case ShowPrintDownloadFailed:
            ShowScreenFor(PrintDownloadFailed); 
            break;
                
        case StartPrintDataLoad:
            ShowScreenFor(LoadingPrintData); 
            break;
            
        case ProcessPrintData:
            ProcessData();
            break;
            
        case ShowPrintDataLoaded:
            ShowScreenFor(LoadedPrintData);
            break;
            
        case StartRegistering:
            _pPrinterStateMachine->process_event(EvConnected());
            break;
            
        case RegistrationSucceeded:
            _homeUISubState = Registered;
            _pPrinterStateMachine->process_event(EvRegistered());
            break;
                      
        case ShowWiFiConnecting:
            ShowScreenFor(WiFiConnecting);
            break;
            
        case ShowWiFiConnectionFailed:
            ShowScreenFor(WiFiConnectionFailed);
            break;

        case ShowWiFiConnected:
            ShowScreenFor(WiFiConnected);
            break;    
            
        case Dismiss:
            _pPrinterStateMachine->process_event(EvDismiss());
            break;
            
        case ShowWhite:
            _projector.ShowWhite();
            break;
            
        case ShowBlack:
            _projector.ShowBlack();
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
        case BTNS_1_AND_2_HOLD:
            // these cases not currently used 
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

// Determines if any delay is needed before exposure.
bool PrintEngine::NeedsPreExposureDelay()
{
    return _cls.ApproachWaitMS > 0;
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
    int numBurnInLayers = _settings.GetInt(BURN_IN_LAYERS);
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

// Increment the current layer number, get any layer-specific settings, 
// and log temperature on the quartiles.
void PrintEngine::NextLayer()
{
    bool retVal = false;
    
    ++_printerStatus._currentLayer;  
    SetEstimatedPrintTime();
    
    GetCurrentLayerSettings();
    
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
        Logger::LogMessage(LOG_INFO, msg); 
    }
}

// Returns true if and only if the current print has more layers to be printed.
bool PrintEngine::MoreLayers()
{
    return _printerStatus._currentLayer < _printerStatus._numLayers;
}

// In a background thread, load the slice image for the next layer, and begin 
// processing it if necessary.
bool PrintEngine::LoadNextLayerImage()
{
    int nextLayer = _printerStatus._currentLayer + 1;
    
    if (!_pPrintData) 
    {
        // if no PrintData available, there's no point in proceeding
        return HandleError(NoImageForLayer, true, NULL, nextLayer);
    }

    // Use background thread to at least load the image into the projector, and
    // possibly perform other image processing.
    
    // First make sure the background thread isn't running already.
    if (_bgndThread != 0)
        return HandleError(IPThreadAlreadyRunning, true);
    
    // copy data needed by the background thread
    _threadData.pImage = &_image;    
    _threadData.pPrintData = _pPrintData.get();
    _threadData.layer = nextLayer;
    _threadData.pProjector = &_projector;
    _threadData.scaleFactor = _settings.GetDouble(IMAGE_SCALE_FACTOR);
    _threadData.usePatternMode = false;
    if(_settings.GetInt(USE_PATTERN_MODE))
    {
        _threadData.scaleFactor = _settings.GetDouble(PAT_MODE_SCALE_FACTOR);
        _threadData.usePatternMode = true;
    }     
    _threadData.imageProcessor = &_imageProcessor;

    _threadError = Success;
    _threadErrorMsg = NULL;

    // start the background thread
    if (pthread_create(&_bgndThread, NULL, &InBackground, &_threadData) != 0)
        return HandleError(CantStartIPThread, true);
    
    return true;
}

// Wait for completion of any processing in background thread.
bool PrintEngine::AwaitEndOfBackgroundThread(bool ignoreErrors)
{
    if (_bgndThread != 0)
    {
        if (pthread_join(_bgndThread, NULL) == 0)
            _bgndThread = 0;
        else if (_threadError == Success)
            _threadError = CantJoinIPThread;
    }
     
    if (_threadError != Success && !ignoreErrors)
    {
        // handle fatal error from background thread
        if (_threadError == NoImageForLayer)
            HandleError(_threadError, true, NULL, _printerStatus._currentLayer);
        else if(_threadError == ImageProcessing)
            HandleError(_threadError, true, _threadErrorMsg);
        else
            HandleError(_threadError, true);
        return false;
    }
    return true;
}

// Sets the estimated print time
void PrintEngine::SetEstimatedPrintTime()
{
    int layersLeft = _printerStatus._numLayers - 
                    (_printerStatus._currentLayer - 1);

    double burnInLayers = _settings.GetInt(BURN_IN_LAYERS);
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
     
// Handles errors with message and optional parameters.
// Returns false for convenience.
bool PrintEngine::HandleError(ErrorCode code, bool fatal, 
                              const char* str, int value)
{
    char* msg;
    int origErrno = errno;
    // log the error
    if (str != NULL)
        msg = Logger::LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, code, 
                                                                          str);
    else if (value != INT_MAX)
        msg = Logger::LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, code, 
                                                                        value);
    else
        msg = Logger::LogError(fatal ? LOG_ERR : LOG_WARNING, origErrno, code);
    
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
        _printerStatus._canLoadPrintData = false;
        // a status update will be sent when we enter the Error state
        _pPrinterStateMachine->HandleFatalError(); 
        // clear error status
        _printerStatus._isError = false;
    }
    return false;
}

// log firmware version, current print status, & current settings
void PrintEngine::LogStatusAndSettings()
{
    Logger::LogMessage(LOG_INFO, (std::string(FW_VERSION_MSG) + 
                                 GetFirmwareVersion()).c_str());
    Logger::LogMessage(LOG_INFO, _printerStatus.ToString().c_str());
    Logger::LogMessage(LOG_INFO, _settings.GetAllSettingsAsJSONString().c_str());    
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
void PrintEngine::SendMotorCommand(HighLevelMotorCommand command)
{
    bool success = true;
        
    switch(command)
    {
        case GoHome:
            success = _motor.GoHome();
            StartMotorTimeoutTimer(GetHomingTimeoutSec());
            break;
            
        case GoHomeWithoutRotateHome:
            success = _motor.GoHome(true, false);
            StartMotorTimeoutTimer(GetHomingTimeoutSec());
            break;
            
        case MoveToStartPosition: 
            success = _motor.GoToStartPosition();
            // for tracking where we are, to enable lifting for inspection
            _currentZPosition = 0;
            StartMotorTimeoutTimer(GetStartPositionTimeoutSec());
            break;
            
        case Separate:
            success = _motor.Separate(_cls);
            StartMotorTimeoutTimer(GetSeparationTimeoutSec());
            break;
                        
        case Approach:
            success = _motor.Approach(_cls);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec());
            break;
            
        case ApproachAfterJam:
            success = _motor.Approach(_cls, true);
            _currentZPosition += _cls.LayerThicknessMicrons;
            StartMotorTimeoutTimer(GetApproachTimeoutSec() +
                                   GetUnjammingTimeoutSec());
            break;
            
        case Press:
            success = _motor.Press(_cls);
            StartMotorTimeoutTimer(GetPressTimeoutSec());
            break;
            
         case UnPress:
            success = _motor.Unpress(_cls);
            StartMotorTimeoutTimer(GetUnpressTimeoutSec());
            break;
 
        case PauseAndInspect:
            success = _motor.PauseAndInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(true));
            break;
            
        case ResumeFromInspect:
            success = _motor.ResumeFromInspect(_cls);
            StartMotorTimeoutTimer(GetPauseAndInspectTimeoutSec(false));
            break;
            
        case RecoverFromJam:
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

// Cleans up from any print in progress.  If withInterrupt, an interrupt will 
// be requested when clearing any pending movement, in case a movement is 
// currently in progress.
void PrintEngine::ClearCurrentPrint(bool withInterrupt)
{
    PauseMovement();
    ClearPendingMovement(withInterrupt);
    
    // log the temperature, for canceled prints or on fatal error
    char msg[50];
    sprintf(msg, LOG_TEMPERATURE, _temperature);
    Logger::LogMessage(LOG_INFO, msg); 
    
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
    _settings.Restore(JOB_ID_SETTING);
    
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

// Wraps Projector's ShowCurrentImage method and handles errors
void PrintEngine::ShowImage()
{
    try
    {
        _projector.ShowCurrentImage();
    }
    catch (const std::exception& e)
    {
        HandleError(CantShowImage, true, NULL, _printerStatus._currentLayer);
    }
}
 
// Wraps Projector's ShowBlack method and handles errors
void PrintEngine::TurnProjectorOff()
{
    try
    {
        _projector.ShowBlack();
    }
    catch (const std::exception& e)
    {
        HandleError(CantShowBlack, true, e.what());
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
    
    // make sure the background thread isn't still running       
    // but ignore any errors from it, which have either already been handled,
    // or will be below when we try to start it again
    AwaitEndOfBackgroundThread(true);
    
    if(!LoadNextLayerImage())
        return false;
   
    // clear per-layer settings in case they exist from a previous print
    _perLayer.Clear();
    
    // use per-layer settings, if print data contains them
    std::string perLayerSettings;
    if (_pPrintData->GetFileContents(PER_LAYER_SETTINGS_FILE, perLayerSettings))
    {
        if (!_perLayer.Load(perLayerSettings))
        {
            HandleError(BadPerLayerSettings, true); 
            return false;
        }      
    }
    
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

// Show a screen for the given UI substate.
bool PrintEngine::ShowScreenFor(UISubState substate)
{
   // These screens can only be shown in the Home or DoorOpen states
    if (_printerStatus._state != HomeState && 
        _printerStatus._state != DoorOpenState)
    {
        HandleError(IllegalStateForUISubState, false, 
                    PrinterStatus::GetStateName(_printerStatus._state));
        return false;
    }

    // arrange to show the appropriate Home screen on the front panel,
    // either now or on returning from DoorOpen
    _homeUISubState = substate;
    
    // set whether or not we can download data
    SetCanLoadPrintData(substate != LoadingPrintData &&
                        substate != DownloadingPrintData);
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
        ShowScreenFor(USBDriveError); 
        return;
    }

    std::ostringstream path;
    path << USB_DRIVE_MOUNT_POINT << "/" << 
                                        _settings.GetString(USB_DRIVE_DATA_DIR);

    PrintFileStorage storage(path.str());

    if (!storage.HasOneFile())
    {
        ShowScreenFor(USBDriveError); 
        return;
    }

    _printerStatus._usbDriveFileName = storage.GetFileName();
    ShowScreenFor(USBDriveFileFound); 
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
        ShowScreenFor(HasAtLeastOneLayer() ? HavePrintData : NoPrintData);
    }
}

// Load the print file from the attached USB drive.
void PrintEngine::LoadPrintFileFromUSBDrive()
{
    ShowScreenFor(LoadingPrintData);

    // copy the file from the USB drive to the download directory
    // print data processing moves or deletes the found file and we don't want
    // to move or delete the user's file from her or his usb drive

    std::ostringstream path;
    path << USB_DRIVE_MOUNT_POINT << "/" << 
                                        _settings.GetString(USB_DRIVE_DATA_DIR);

    PrintFileStorage storage(path.str());

    Copy(storage.GetFilePath(), _settings.GetString(DOWNLOAD_DIR));

    ProcessData();
}

// Prepare downloaded print data for printing.
// Looks for print file in specified directory.
void PrintEngine::ProcessData()
{
    PrintFileStorage storage(_settings.GetString(DOWNLOAD_DIR));
    
    // If any processing step fails, clear downloading screen, report an error,
    // and return to prevent any further processing

    // construct an instance of a PrintData object using a file from the 
    // download directory
    boost::scoped_ptr<PrintData> pNewPrintData(PrintData::CreateFromNewData(
            storage, _settings.GetString(STAGING_DIR),
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
    if (!_settings.RestoreAllPrintSettings())
        // error logged in Settings
        return;

    bool settingsLoaded = false;

    // determine if a temp settings file exists, containing settings for 
    // incoming data
    if (std::ifstream(TEMP_SETTINGS_FILE))
        // use settings from temp file
        settingsLoaded = _settings.SetFromFile(TEMP_SETTINGS_FILE);
    else
    {
        // use settings from file contained in print data
        std::string settings;
        if (pNewPrintData->GetFileContents(EMBEDDED_PRINT_SETTINGS_FILE, 
                                                                    settings))
            settingsLoaded = _settings.SetFromJSONString(settings);
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

    // try to set the appropriate mode
    if (!SetPrintMode())
    {
        HandleProcessDataFailed(_settings.GetInt(USE_PATTERN_MODE) ? 
                                PatternModeError : VideoModeError, 
                                storage.GetFileName());
        return;
    }
    
    // move the new print data from the staging directory to the print data 
    // directory
    if (!pNewPrintData->Move(_settings.GetString(PRINT_DATA_DIR)))
    {
        HandleProcessDataFailed(CantMovePrintData, storage.GetFileName());
        return;
    }

    // Update PrintEngine's reference so it points to the newly processed print 
    // data.  After the swap, the smart pointer pNewPrintData will delete the 
    // "old" print data instance when it goes out of scope and the _pPrintData 
    // member variable will point to the "new" print data instance.
    _pPrintData.swap(pNewPrintData);
    
    // record the name of the last file downloaded
    _settings.Set(PRINT_FILE_SETTING, storage.GetFileName());
    _settings.Save();
   
    // update the printer status with the job id
    _printerStatus._jobID = _settings.GetString(JOB_ID_SETTING);
        
    ShowScreenFor(LoadedPrintData);
}

// Convenience method handles the error and sends status update with
// UISubState needed to show that processing data failed on the front panel
// (unless we're already showing an error)
// Also ensures removal of temp settings file and any settings that would
// otherwise indicate the presence of valid print data.
void PrintEngine::HandleProcessDataFailed(ErrorCode errorCode, 
                                          const std::string& jobName)
{
    if (std::ifstream(TEMP_SETTINGS_FILE))
        remove(TEMP_SETTINGS_FILE);

    // clear print data settings that may have been set by the attempted load
    _settings.RestoreAllPrintSettings();
    if (_pPrintData) 
        ClearPrintData();
    
    HandleError(errorCode, false, jobName.c_str());
    _homeUISubState = PrintDataLoadFailed;
    // don't send new status if we're already showing a fatal error
    if (_printerStatus._state != ErrorState)
        SendStatus(_printerStatus._state, NoChange, PrintDataLoadFailed);
}

// Delete any existing printable data.
void PrintEngine::ClearPrintData()
{
    if (_pPrintData) 
    {
        _pPrintData->Remove();
        ClearHomeUISubState();
        // also clear job name, ID, and last print file
        _settings.Restore(JOB_NAME_SETTING);
        _settings.Restore(PRINT_FILE_SETTING);
        ClearJobID();   
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
    double height = _settings.GetInt(LAYER_THICKNESS);
       
    switch(type)
    {
        case First:
            // start with the exposure time, in seconds
            time = (double) _settings.GetDouble(FIRST_EXPOSURE);
            // plus additional delay (converted from ms)
            time += _settings.GetInt(FL_APPROACH_WAIT) / 1000.0;
            // add separation time
            revs = _settings.GetInt(FL_ROTATION) / MILLIDEGREES_PER_REV;
            // rotation speeds in RPM, convert to revs per sec
            time += (revs / _settings.GetInt(FL_SEPARATION_R_SPEED)) * 60.0;
            // Z speeds are in microns/s
            zLift = _settings.GetInt(FL_Z_LIFT);
            time += zLift / _settings.GetInt(FL_SEPARATION_Z_SPEED);
            // add approach time
            time += (revs / _settings.GetInt(FL_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / _settings.GetInt(FL_APPROACH_Z_SPEED);
            // add press/delay/unpress times, if tray deflection used
            press = _settings.GetInt(FL_PRESS);
            if (press != 0)
            {
                time += press / _settings.GetInt(FL_PRESS_SPEED);
                time += _settings.GetInt(FL_PRESS_WAIT) / 1000.0;
                time += press / _settings.GetInt(FL_UNPRESS_SPEED);
            }
            break;
            
        case BurnIn:
            time = (double) _settings.GetDouble(BURN_IN_EXPOSURE);
            time += _settings.GetInt(BI_APPROACH_WAIT) / 1000.0;   
            revs = _settings.GetInt(BI_ROTATION) / MILLIDEGREES_PER_REV;
            time += (revs / _settings.GetInt(BI_SEPARATION_R_SPEED)) * 60.0;
            zLift = _settings.GetInt(BI_Z_LIFT);
            time += zLift / _settings.GetInt(BI_SEPARATION_Z_SPEED);
            time += (revs / _settings.GetInt(BI_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / _settings.GetInt(BI_APPROACH_Z_SPEED);
            press = _settings.GetInt(BI_PRESS);
            if (press != 0)
            {
                time += press / _settings.GetInt(BI_PRESS_SPEED);
                time += _settings.GetInt(BI_PRESS_WAIT) / 1000.0;
                time += press / _settings.GetInt(BI_UNPRESS_SPEED);
            }
            break;
            
        case Model:
            time = (double) _settings.GetDouble(MODEL_EXPOSURE);
            time += _settings.GetInt(ML_APPROACH_WAIT) / 1000.0;    
            revs = _settings.GetInt(ML_ROTATION) / MILLIDEGREES_PER_REV;
            time += (revs / _settings.GetInt(ML_SEPARATION_R_SPEED)) * 60.0;
            zLift = _settings.GetInt(ML_Z_LIFT);
            time += zLift / _settings.GetInt(ML_SEPARATION_Z_SPEED);
            time += (revs / _settings.GetInt(ML_APPROACH_R_SPEED)) * 60.0;    
            time += (zLift - height) / _settings.GetInt(ML_APPROACH_Z_SPEED);
            press = _settings.GetInt(ML_PRESS);
            if (press != 0)
            {
                time += press / _settings.GetInt(ML_PRESS_SPEED);
                time += _settings.GetInt(ML_PRESS_WAIT) / 1000.0;
                time += press / _settings.GetInt(ML_UNPRESS_SPEED);
            }

            break; 
    }
    
    // add measured overhead 
    time += _settings.GetDouble(LAYER_OVERHEAD);
    
    return time;   
}

// Checks to see if the printer is too hot to function
bool PrintEngine::IsPrinterTooHot()
{
    _alreadyOverheated = false;
    if (_temperature > _settings.GetDouble(MAX_TEMPERATURE))
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
    if (_settings.GetInt(DETECT_JAMS) == 0 || // jam detection disabled or
       _settings.GetInt(HARDWARE_REV) == 0)   // old hardware lacking this sensor
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
        StartMotorTimeoutTimer((int)_settings.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
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

// Determines if any delay after tray deflection is needed.
bool PrintEngine::NeedsTrayDeflectionPause()
{
    return _cls.PressWaitMS > 0;
}

// Pad the raw expected time for a movement to get a reasonable timeout period.
int  PrintEngine::PadTimeout(double rawTime)
{
    return (int) (rawTime * _settings.GetDouble(MOTOR_TIMEOUT_FACTOR) + 
                            _settings.GetDouble(MIN_MOTOR_TIMEOUT_SEC));
}

// Returns the timeout (in seconds) to allow for getting to the home position
int PrintEngine::GetHomingTimeoutSec()
{
    double rSpeed = _settings.GetInt(R_HOMING_SPEED);
    double zSpeed = _settings.GetInt(Z_HOMING_SPEED);
     
    double deltaR = 1;  // may take up to one full revolution
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    // Z height is in microns and speed in microns/s
    return PadTimeout(deltaR / rSpeed + 
                      abs(_settings.GetInt(Z_START_PRINT_POSITION)) / zSpeed);   
}

// Returns the timeout (in seconds) to allow for getting to the start position
int PrintEngine::GetStartPositionTimeoutSec()
{
    double rSpeed = _settings.GetInt(R_START_PRINT_SPEED);
    double zSpeed = _settings.GetInt(Z_START_PRINT_SPEED);

    double deltaR = _settings.GetInt(R_START_PRINT_ANGLE);
    // convert to revolutions
    deltaR /= MILLIDEGREES_PER_REV;
    // rSpeed is in RPM, convert to revolutions per second
    rSpeed /= 60.0;
    // Z height is in microns and speed in microns/s
    
    return GetHomingTimeoutSec() +          // we also need to go home first
           PadTimeout(deltaR / rSpeed +  
                      abs(_settings.GetInt(Z_START_PRINT_POSITION)) / zSpeed);   
}

// Returns the timeout (in seconds) to allow for moving to or from the pause 
// and inspect position.
int PrintEngine::GetPauseAndInspectTimeoutSec(bool toInspect)
{   
    double zSpeed, rSpeed;
    
    if (toInspect)
    {
        // moving up uses homing speeds
        rSpeed = _settings.GetInt(R_HOMING_SPEED);
        zSpeed = _settings.GetInt(Z_HOMING_SPEED);
    }
    else
    {
        // moving down uses start print speeds
        rSpeed = _settings.GetInt(R_START_PRINT_SPEED);
        zSpeed = _settings.GetInt(Z_START_PRINT_SPEED);
    }
      
    // convert to revolutions
    double revs = _cls.RotationMilliDegrees / MILLIDEGREES_PER_REV;
    // rSpeed is in RPM, convert to revolutions per second
    // Z height is in microns and speed in microns/s
    return PadTimeout((revs / rSpeed) * 60.0 +  
                      _settings.GetInt(INSPECTION_HEIGHT) / zSpeed);
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
// called once per layer, after the layer number has been incremented 
// for it.
void PrintEngine::GetCurrentLayerSettings()
{
    // The settings after exposure use the same layer type, but use the number
    // of the next layer for any per-layer overrides.
    int n = GetCurrentLayerNum();
    int p = n + 1;
    
    // find the type of layer n
    LayerType type = Model;
    if (n == 1)
        type = First;
    else
    {
        int numBurnInLayers = _settings.GetInt(BURN_IN_LAYERS);
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
    _cls.InspectionHeightMicrons = _settings.GetInt(INSPECTION_HEIGHT);
    // see if there's enough headroom to lift the model for inspection.
    _cls.CanInspect = (_cls.InspectionHeightMicrons != 0) && 
                      (_settings.GetInt(MAX_Z_TRAVEL) > (_currentZPosition +  
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
    if (!_haveHardware || _settings.GetInt(HARDWARE_REV) == 0)
        return false;
    
    static bool firstTime = true;
    
    if (firstTime)
    {
        firstTime = false;  // only do this once
        
        // read the GPIO connected to the front panel button        
        // setup GPIO as input pin
        char GPIOInputString[4], GPIOInputValue[64], GPIODirection[64], 
             setValue[10], value;
        FILE* inputHandle = NULL;

        // setup input
        sprintf(GPIOInputString, "%d", BUTTON2_DIRECT);
        sprintf(GPIOInputValue, GPIO_VALUE, BUTTON2_DIRECT);
        sprintf(GPIODirection, GPIO_DIRECTION, BUTTON2_DIRECT);

        // export & configure the pin
        if ((inputHandle = fopen(GPIO_EXPORT, "ab")) == NULL)
        {
            Logger::LogError(LOG_ERR, errno, GpioExport, BUTTON2_DIRECT);
            return false;
        }
        strcpy(setValue, GPIOInputString);
        fwrite(&setValue, sizeof(char), 2, inputHandle);
        fclose(inputHandle);

        // Set direction of the pin to an input
        if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL)
        {
            Logger::LogError(LOG_ERR, errno, GpioDirection, BUTTON2_DIRECT);
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
            Logger::LogError(LOG_ERR, errno, GpioInput, BUTTON2_DIRECT);
            return false;
        }  

        read(fd, &value, 1);
        _demoModeRequested = (value == '0');
        close(fd);
        
        // Unexport the pin
        if ((inputHandle = fopen(GPIO_UNEXPORT, "ab")) == NULL) 
        {
            Logger::LogError(LOG_ERR, errno, GpioUnexport);
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
    _motor.GoHome(true, true, true);  
    // (and leave the motors enabled to hold their positions)
   
    try
    {
        _projector.ShowWhite();
    }
    catch (const std::exception& e)
    {
        HandleError(CantShowWhite, true, e.what());
    }
}

ErrorCode PrintEngine::_threadError = Success;
const char* PrintEngine::_threadErrorMsg = NULL;

// Perform processing in a background thread.  Do not access Settings here,
// as they are not thread safe.
void* PrintEngine::InBackground(void* context)
{
    try
    {
        // make this thread high priority
        pid_t tid = syscall(SYS_gettid);
        setpriority(PRIO_PROCESS, tid, -10); 

        ThreadData* pData = (ThreadData*)context;

        if (!pData->pPrintData->GetImageForLayer(pData->layer, pData->pImage))
        {
            _threadError = NoImageForLayer;
            pthread_exit(NULL);
        }

        // do image scaling if needed
        if (pData->scaleFactor != 1.0)
            pData->imageProcessor->Scale(pData->pImage, pData->scaleFactor);
        
        Magick::Image* pOutput = pData->pImage;
        // remap the image for pattern mode if needed
        if (pData->usePatternMode)
            pOutput = pData->imageProcessor->MapForPatternMode(*pData->pImage);
        
        // convert the image to a projectable format
        pData->pProjector->SetImage(*pOutput);
    }
    catch (const std::exception& e)
    {
        _threadError = ImageProcessing;
        _threadErrorMsg = e.what(); 
    }
    
    pthread_exit(NULL);
}

// Set or clear PrinterStatus flag indicating if we can load print data.
void PrintEngine::SetCanLoadPrintData(bool canLoad)
{
    _printerStatus._canLoadPrintData = canLoad;
}

// Put the projector into or out of ProgramMode
bool PrintEngine::PutProjectorInProgramMode(bool enter)
{
    bool retVal = _projector.EnterProgramMode(enter);
    
    if(enter)
    {
        // allow time for projector controller to jump to boot-loader program
        StartDelayTimer(5);
        // use layers to indicate progress of upgrade process
        _printerStatus._numLayers = NUM_LEDS_IN_RING;
    }
}

// Tell the projector to upgrade its firmware.
void PrintEngine::UpgradeProjectorFirmware()
{
    if (!_projector.UpgradeFirmware())
        HandleError(ProjectorUpgradeError, true);
    else if (_projector.ProgrammingComplete())
        _pPrinterStateMachine->process_event(EvUpgadeCompleted()); 
    else 
    {
        // get upgrade progress  and report it, 
        // as if it was the layer of a print in progress
        _printerStatus._currentLayer = (int) (_projector.GetUpgradeProgress() * 
                                              NUM_LEDS_IN_RING + 0.5);
        if (_printerStatus._currentLayer > NUM_LEDS_IN_RING)
            _printerStatus._currentLayer = NUM_LEDS_IN_RING;
    }
}

// Allow the state machine to determine when programming is complete, to avoid
// sending a "programming in progress" status update after it's already done.
bool PrintEngine::ProjectorProgrammingCompleted() 
{ 
    return _projector.ProgrammingComplete();
}

// Set the projector into video or pattern mode, depending on the current 
// setting, and set the corresponding video resolution.  Returns true if and 
// only if the mode could be set (or if we are already in the requested mode).
bool PrintEngine::SetPrintMode()
{
    // see if we need to switch to/from pattern mode
    if(_settings.GetInt(USE_PATTERN_MODE))
    {
        if (!_projector.SetPatternMode())
        {
            HandleError(PatternModeError, true); 
            return false;
        }      
    }
    else if(!_projector.SetVideoMode())
    {
        HandleError(VideoModeError, true); 
        return false;            
    }
    return true;
}