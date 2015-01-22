/* 
 * File:   PrinterStatus.cpp
 * Author: Richard Greene
 * 
 * The data structure used to communicate status from the print engine to UI 
 * components.
 *
 * Created on July 28, 2014, 5:25 PM
 */

#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if(x);                                            \
  else throw std::exception();  

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <PrinterStatus.h>
#include <Logger.h>
#include <Shared.h>
#include <Settings.h>

using namespace rapidjson;

/// Constructor
PrinterStatus::PrinterStatus() :
_state(PrinterOnState),
_change(NoChange),
_UISubState(NoUISubState),
_isError(false),
_errorCode(Success),
_errno(0),
_numLayers(0),
_currentLayer(0),
_estimatedSecondsRemaining(0),
_temperature(0.0)
{
}

/// Gets the name of a print engine state machine state
const char* PrinterStatus::GetStateName(PrintEngineState state)
{
    static bool initialized = false;
    static const char* stateNames[MaxPrintEngineState];
    if(!initialized)
    {
        // initialize the array of state names
        stateNames[PrinterOnState] = PRINTER_ON_STATE;
        stateNames[DoorClosedState] = DOOR_CLOSED_STATE;
        stateNames[InitializingState] = INITIALIZING_STATE;
        stateNames[DoorOpenState] = DOOR_OPEN_STATE;
        stateNames[HomingState] = HOMING_STATE;
        stateNames[HomeState] = HOME_STATE;
        stateNames[ErrorState] = ERROR_STATE;
        stateNames[PrintSetupState] = PRINT_SETUP_STATE;
        stateNames[MovingToStartPositionState] = MOVING_TO_START_POSITION_STATE;
        stateNames[ExposingState] = EXPOSING_STATE;
        stateNames[PrintingLayerState] = PRINTING_LAYER_STATE;
        stateNames[RotatingForPauseState] = ROTATING_FOR_PAUSE_STATE;
        stateNames[MovingToPauseState] = MOVING_TO_PAUSE_STATE;
        stateNames[PausedState] = PAUSED_STATE;
        stateNames[RotatingForResumeState] = ROTATING_FOR_RESUME_STATE;
        stateNames[MovingToResumeState] = MOVING_TO_RESUME_STATE;
        stateNames[SeparatingState] = SEPARATING_STATE;
        stateNames[ConfirmCancelState] = CONFIRM_CANCEL_STATE;
        stateNames[ShowingVersionState] = SHOWING_VERSION_STATE;
        stateNames[CalibrateState] = CALIBRATE_STATE;
        stateNames[MovingToCalibrationState] = MOVING_TO_CALIBRATION_STATE;
        stateNames[CalibratingState] = CALIBRATING_STATE;
        stateNames[EndingCalibrationState] = ENDING_CALIBRATION_STATE;
        stateNames[RegisteringState] = REGISTERING_STATE;
        stateNames[JammedState] = JAMMED_STATE;
        
        initialized = true;
    }
    
    if(state <= UndefinedPrintEngineState || state >= MaxPrintEngineState)
    {
        LOGGER.HandleError(UnknownPrintEngineState, false, NULL, state);
        return "";                                                              
    }
    return stateNames[state];
}

/// Gets the name of a print engine state machine UI sub-state
const char* PrinterStatus::GetSubStateName(UISubState substate)
{
    static bool initialized = false;
    static const char* substateNames[MaxUISubState];
    if(!initialized)
    {
        // initialize the array of state names
        substateNames[NoUISubState] = NO_SUBSTATE;
        substateNames[NoPrintData] = NO_PRINT_DATA_SUBSTATE;
        substateNames[DownloadingPrintData] = DOWNLOADING_PRINT_DATA_SUBSTATE;
        substateNames[PrintDownloadFailed] = PRINT_DOWNLOAD_FAILED_SUBSTATE;
        substateNames[LoadingPrintData] = LOADING_PRINT_DATA_SUBSTATE;
        substateNames[LoadedPrintData] = LOADED_PRINT_DATA_SUBSTATE;
        substateNames[PrintDataLoadFailed] = PRINT_DATA_LOAD_FAILED_SUBSTATE;
        substateNames[HavePrintData] = HAVE_PRINT_DATA_SUBSTATE;
        substateNames[PrintCanceled] = PRINT_CANCELED_SUBSTATE;
        substateNames[PrintCompleted] = PRINT_COMPLETED_SUBSTATE;
        substateNames[ExitingDoorOpen] = EXITING_DOOR_OPEN_SUBSTATE;
        substateNames[Registered] = REGISTERED_SUBSTATE;
        substateNames[AboutToPause] = ABOUT_TO_PAUSE_SUBSTATE;
        substateNames[WiFiConnecting] = WIFI_CONNECTING_SUBSTATE;
        substateNames[WiFiConnectionFailed] = WIFI_CONNECTION_FAILED_SUBSTATE;
        substateNames[WiFiConnected] = WIFI_CONNECTED_SUBSTATE;
        
        initialized = true;
    }
    
    if(substate < NoUISubState || substate >= MaxUISubState)
    {
        LOGGER.HandleError(UnknownPrintEngineSubState, false, NULL, substate);
        return "";                                                              
    }
    return substateNames[substate];
}

/// Returns printer status as a JSON formatted string.
std::string PrinterStatus::ToString()
{
    std::string retVal = "";
    
    try
    {
        const char json[] = "{"
            "\"" STATE_PS_KEY "\": \"\","
            "\"" UISUBSTATE_PS_KEY "\": \"\","   
            "\"" CHANGE_PS_KEY "\": \"\","
            "\"" IS_ERROR_PS_KEY "\": false,"
            "\"" ERROR_CODE_PS_KEY "\": 0,"
            "\"" ERRNO_PS_KEY "\": 0,"
            "\"" ERROR_MSG_PS_KEY "\": \"\","
            "\"" JOB_NAME_PS_KEY "\": \"\","      
            "\"" JOB_ID_PS_KEY "\": \"\","      
            "\"" LAYER_PS_KEY "\": 0,"
            "\"" TOAL_LAYERS_PS_KEY "\": 0,"
            "\"" SECONDS_LEFT_PS_KEY "\": 0,"
            "\"" TEMPERATURE_PS_KEY "\": 0.0"
        "}"; 
 
        Document doc;
        doc.Parse(json);
        
        Value s;
        const char* str = STATE_NAME(_state);
        s.SetString(str, strlen(str), doc.GetAllocator());       
        doc[STATE_PS_KEY] = s; 
        
        str = SUBSTATE_NAME(_UISubState);
        s.SetString(str, strlen(str), doc.GetAllocator()); 
        doc[UISUBSTATE_PS_KEY] = s;        
        
        s = NO_CHANGE;
        if(_change == Entering)
           s = ENTERING;
        else if(_change == Leaving)
           s = LEAVING;
        doc[CHANGE_PS_KEY] = s; 
        
        doc[IS_ERROR_PS_KEY] = _isError;        
        doc[ERROR_CODE_PS_KEY] = _errorCode; 
        doc[ERRNO_PS_KEY] = _errno; 
        s.SetString(GetLastErrorMessage().c_str(), 
                    GetLastErrorMessage().size(), doc.GetAllocator()); 
        doc[ERROR_MSG_PS_KEY] = s;       
        
        // job name and ID come from settings rather than PrinterStatus
        std::string ss = SETTINGS.GetString(JOB_NAME_SETTING);
        s.SetString(ss.c_str(), ss.size(), doc.GetAllocator()); 
        doc[JOB_NAME_PS_KEY] = s;        
        
        ss = SETTINGS.GetString(JOB_ID_SETTING);
        s.SetString(ss.c_str(), ss.size(), doc.GetAllocator()); 
        doc[JOB_ID_PS_KEY] = s;        
        
        doc[LAYER_PS_KEY] = _currentLayer;
        doc[TOAL_LAYERS_PS_KEY] = _numLayers;
        doc[SECONDS_LEFT_PS_KEY] = _estimatedSecondsRemaining;
        doc[TEMPERATURE_PS_KEY] = _temperature;
        
        
        StringBuffer buffer; 
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);        
        retVal = std::string(buffer.GetString()) + "\n";
    }
    catch(std::exception)
    {
        LOGGER.HandleError(PrinterStatusToString);
    }
    return retVal; 
}

std::string _lastErrorMessage = "";

/// Static method to set the one and only last error message.
void PrinterStatus::SetLastErrorMsg(std::string msg)
{
    _lastErrorMessage = msg;
}

/// Static method to return the one and only last error message.
std::string PrinterStatus::GetLastErrorMessage()
{
    return _lastErrorMessage;
}