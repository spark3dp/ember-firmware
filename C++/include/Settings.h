//  File:   Settings.h
//  Handles storage, retrieval, and reset of settings
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#ifndef SETTINGS_H
#define	SETTINGS_H

#include <string>
#include <set>

#include <rapidjson/document.h>

#include "IErrorHandler.h"

using namespace rapidjson;

// setting name strings
constexpr const char* JOB_NAME_SETTING       = "JobName";
constexpr const char* USER_NAME_SETTING      = "UserName";
//constexpr const char* JOB_ID_SETTING        "JobID"     // defined in shared.h
//constexpr const char* PRINT_FILE_SETTING    "PrintFile" // defined in shared.h
constexpr const char* LAYER_THICKNESS        = "LayerThicknessMicrons";
constexpr const char* BURN_IN_LAYERS         = "BurnInLayers";
constexpr const char* FIRST_EXPOSURE         = "FirstExposureSec";
constexpr const char* BURN_IN_EXPOSURE       = "BurnInExposureSec";
constexpr const char* MODEL_EXPOSURE         = "ModelExposureSec";
constexpr const char* PRINT_DATA_DIR         = "PrintDataDir";
constexpr const char* DOWNLOAD_DIR           = "DownloadDir";
constexpr const char* STAGING_DIR            = "StagingDir";
constexpr const char* HARDWARE_REV           = "HardwareRev";
constexpr const char* LAYER_OVERHEAD         = "LayerExtraSec";
constexpr const char* MAX_TEMPERATURE        = "MaxTemperatureC";
constexpr const char* DETECT_JAMS            = "DetectJams";
constexpr const char* MAX_UNJAM_TRIES        = "MaxUnjamTries";
constexpr const char* MOTOR_TIMEOUT_FACTOR   = "MotorTimeoutScaleFactor";
constexpr const char* MIN_MOTOR_TIMEOUT_SEC  = "MinMotorTimeoutSec";
constexpr const char* PROJECTOR_LED_CURRENT  = "ProjectorLEDCurrent";
constexpr const char* FRONT_PANEL_AWAKE_TIME = "FrontPanelScreenSaverMinutes";
constexpr const char* IMAGE_SCALE_FACTOR     = "ImageScaleFactor";
constexpr const char* PAT_MODE_SCALE_FACTOR  = "PatternModeImageScaleFactor";
constexpr const char* USB_DRIVE_DATA_DIR     = "USBDriveDataDir";
constexpr const char* FW_VERSION             = "FirmwareVersion";

// motor control settings for moving between layers
// FL = first layer, BI = burn-in layer, ML = model Layer
constexpr const char* FL_SEPARATION_R_JERK   = "FirstSeparationRotJerk";
constexpr const char* FL_SEPARATION_R_SPEED  = "FirstSeparationRPM";
constexpr const char* FL_APPROACH_R_JERK     = "FirstApproachRotJerk";
constexpr const char* FL_APPROACH_R_SPEED    = "FirstApproachRPM";
constexpr const char* FL_Z_LIFT              = "FirstZLiftMicrons";
constexpr const char* FL_SEPARATION_Z_JERK   = "FirstSeparationZJerk";
constexpr const char* FL_SEPARATION_Z_SPEED  = "FirstSeparationMicronsPerSec";
constexpr const char* FL_APPROACH_Z_JERK     = "FirstApproachZJerk";
constexpr const char* FL_APPROACH_Z_SPEED    = "FirstApproachMicronsPerSec";
constexpr const char* FL_ROTATION            = "FirstRotationMilliDegrees";
constexpr const char* FL_EXPOSURE_WAIT       = "FirstExposureWaitMS";
constexpr const char* FL_SEPARATION_WAIT     = "FirstSeparationWaitMS";
constexpr const char* FL_APPROACH_WAIT       = "FirstApproachWaitMS";
constexpr const char* FL_PRESS               = "FirstPressMicrons";
constexpr const char* FL_PRESS_SPEED         = "FirstPressMicronsPerSec";
constexpr const char* FL_PRESS_WAIT          = "FirstPressWaitMS";
constexpr const char* FL_UNPRESS_SPEED       = "FirstUnPressMicronsPerSec";

constexpr const char* BI_SEPARATION_R_JERK   = "BurnInSeparationRotJerk";
constexpr const char* BI_SEPARATION_R_SPEED  = "BurnInSeparationRPM";
constexpr const char* BI_APPROACH_R_JERK     = "BurnInApproachRotJerk";
constexpr const char* BI_APPROACH_R_SPEED    = "BurnInApproachRPM";
constexpr const char* BI_Z_LIFT              = "BurnInZLiftMicrons";
constexpr const char* BI_SEPARATION_Z_JERK   = "BurnInSeparationZJerk";
constexpr const char* BI_SEPARATION_Z_SPEED  = "BurnInSeparationMicronsPerSec";
constexpr const char* BI_APPROACH_Z_JERK     = "BurnInApproachZJerk";
constexpr const char* BI_APPROACH_Z_SPEED    = "BurnInApproachMicronsPerSec";
constexpr const char* BI_ROTATION            = "BurnInRotationMilliDegrees";
constexpr const char* BI_EXPOSURE_WAIT       = "BurnInExposureWaitMS";
constexpr const char* BI_SEPARATION_WAIT     = "BurnInSeparationWaitMS";
constexpr const char* BI_APPROACH_WAIT       = "BurnInApproachWaitMS";
constexpr const char* BI_PRESS               = "BurnInPressMicrons";
constexpr const char* BI_PRESS_SPEED         = "BurnInPressMicronsPerSec";
constexpr const char* BI_PRESS_WAIT          = "BurnInPressWaitMS";
constexpr const char* BI_UNPRESS_SPEED       = "BurnInUnPressMicronsPerSec";

constexpr const char* ML_SEPARATION_R_JERK   = "ModelSeparationRotJerk";
constexpr const char* ML_SEPARATION_R_SPEED  = "ModelSeparationRPM";
constexpr const char* ML_APPROACH_R_JERK     = "ModelApproachRotJerk";
constexpr const char* ML_APPROACH_R_SPEED    = "ModelApproachRPM";
constexpr const char* ML_Z_LIFT              = "ModelZLiftMicrons";
constexpr const char* ML_SEPARATION_Z_JERK   = "ModelSeparationZJerk";
constexpr const char* ML_SEPARATION_Z_SPEED  = "ModelSeparationMicronsPerSec";
constexpr const char* ML_APPROACH_Z_JERK     = "ModelApproachZJerk";
constexpr const char* ML_APPROACH_Z_SPEED    = "ModelApproachMicronsPerSec";
constexpr const char* ML_ROTATION            = "ModelRotationMilliDegrees";
constexpr const char* ML_EXPOSURE_WAIT       = "ModelExposureWaitMS";
constexpr const char* ML_SEPARATION_WAIT     = "ModelSeparationWaitMS";
constexpr const char* ML_APPROACH_WAIT       = "ModelApproachWaitMS";
constexpr const char* ML_PRESS               = "ModelPressMicrons";
constexpr const char* ML_PRESS_SPEED         = "ModelPressMicronsPerSec";
constexpr const char* ML_PRESS_WAIT          = "ModelPressWaitMS";
constexpr const char* ML_UNPRESS_SPEED       = "ModelUnPressMicronsPerSec";

// settings for pause & inspect
constexpr const char* INSPECTION_HEIGHT      = "InspectionHeightMicrons";
constexpr const char* MAX_Z_TRAVEL           = "MaxZTravelMicrons";

// settings for initializing motor controller
constexpr const char* MICRO_STEPS_MODE       = "MicroStepsMode";

constexpr const char* Z_STEP_ANGLE           = "ZStepAngleMillidegrees";
constexpr const char* Z_MICRONS_PER_REV      = "ZMicronsPerMotorRev";

constexpr const char* R_STEP_ANGLE           = "RStepAngleMillidegrees";
constexpr const char* R_MILLIDEGREES_PER_REV = "RMilliDegreesPerMotorRev";

// motor control settings for homing
constexpr const char* Z_HOMING_JERK          = "ZHomingJerk";
constexpr const char* Z_HOMING_SPEED         = "ZHomingSpeedMicronsPerSec";
constexpr const char* R_HOMING_JERK          = "RHomingJerk";
constexpr const char* R_HOMING_SPEED         = "RHomingSpeedRPM";
constexpr const char* R_HOMING_ANGLE         = "RHomingAngleMilliDegrees";

// motor control settings for starting a print/calibrating
constexpr const char* Z_START_PRINT_JERK     = "ZStartPrintJerk";
constexpr const char* Z_START_PRINT_SPEED    = "ZStartPrintSpeedMicronsPerSec";
constexpr const char* Z_START_PRINT_POSITION = "ZStartPositionMicrons";
constexpr const char* R_START_PRINT_JERK     = "RStartPrintJerk";
constexpr const char* R_START_PRINT_SPEED    = "RStartPrintSpeedRPM";
constexpr const char* R_START_PRINT_ANGLE    = "RStartPrintPositionMillidegrees";

constexpr const char* HOME_ON_APPROACH       = "RotateHomeOnApproach";
constexpr const char* USE_PATTERN_MODE       = "UsePatternMode";

// The class that handles configuration and print options
class Settings 
{
public:
    Settings(const std::string& path);
    virtual ~Settings();
    bool Load(const std::string& filename, bool initializing = false);
    void Save(const std::string& filename);
    void Save();
    void RestoreAll();
    bool RestoreAllPrintSettings();
    void Restore(const std::string key);
    void Refresh();
    void Set(const std::string key, const std::string value);
    void Set(const std::string key, int value);
    void Set(const std::string key, double value);
    int GetInt(const std::string key);
    std::string GetString(const std::string key);
    double GetDouble(const std::string key);
    void SetErrorHandler(IErrorHandler* handler) { _errorHandler = handler; }
    std::string GetAllSettingsAsJSONString();
    bool SetFromJSONString(const std::string& str);
    bool SetFromFile(const std::string& filename);
    
protected:
    std::string _settingsPath;
    std::set<std::string> _names;
    IErrorHandler* _errorHandler;
    
    bool IsValidSettingName(const std::string key);
    void EnsureSettingsDirectoryExists();
    bool AreSameType(Value& a, Value& b);
    bool HandleError(ErrorCode code, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX);
    Document _settingsDoc;
    std::string _defaultJSON;
    std::string _defaultPrintSpecificJSON;
};

// Singleton for sharing settings among all components
class PrinterSettings : public Settings
{
public:
    static Settings& Instance();
    
private:
    PrinterSettings();
    PrinterSettings(const std::string& path);
    PrinterSettings(PrinterSettings const&);
    PrinterSettings& operator=(PrinterSettings const&);
    ~PrinterSettings();
};
#endif    // SETTINGS_H

