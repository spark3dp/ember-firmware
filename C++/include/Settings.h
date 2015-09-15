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

#include <Logger.h>

using namespace rapidjson;

#define SETTINGS (PrinterSettings::Instance())

// setting name strings
#define JOB_NAME_SETTING        "JobName"
//#define JOB_ID_SETTING        "JobID"      // defined in shared.h
//#define PRINT_FILE_SETTING    "PrintFile"  // defined in shared.h
#define LAYER_THICKNESS         "LayerThicknessMicrons"
#define BURN_IN_LAYERS          "BurnInLayers"
#define FIRST_EXPOSURE          "FirstExposureSec"
#define BURN_IN_EXPOSURE        "BurnInExposureSec"
#define MODEL_EXPOSURE          "ModelExposureSec"
#define PRINT_DATA_DIR          "PrintDataDir"
#define DOWNLOAD_DIR            "DownloadDir"
#define STAGING_DIR             "StagingDir"
#define HARDWARE_REV            "HardwareRev"
#define LAYER_OVERHEAD          "LayerExtraSec"
#define MAX_TEMPERATURE         "MaxTemperatureC"
#define DETECT_JAMS             "DetectJams"
#define MAX_UNJAM_TRIES         "MaxUnjamTries"
#define MOTOR_TIMEOUT_FACTOR    "MotorTimeoutScaleFactor"          
#define MIN_MOTOR_TIMEOUT_SEC   "MinMotorTimeoutSec"
#define PROJECTOR_LED_CURRENT   "ProjectorLEDCurrent"
#define FRONT_PANEL_AWAKE_TIME  "FrontPanelScreenSaverMinutes"
#define IMAGE_SCALE_FACTOR      "ImageScaleFactor"
#define USB_DRIVE_DATA_DIR      "USBDriveDataDir"

// motor control settings for moving between layers
// FL = first layer, BI = burn-in layer, ML = model Layer
#define FL_SEPARATION_R_JERK    "FirstSeparationRotJerk"
#define FL_SEPARATION_R_SPEED   "FirstSeparationRPM"
#define FL_APPROACH_R_JERK      "FirstApproachRotJerk"
#define FL_APPROACH_R_SPEED     "FirstApproachRPM"
#define FL_Z_LIFT               "FirstZLiftMicrons"
#define FL_SEPARATION_Z_JERK    "FirstSeparationZJerk"
#define FL_SEPARATION_Z_SPEED   "FirstSeparationMicronsPerSec"
#define FL_APPROACH_Z_JERK      "FirstApproachZJerk"
#define FL_APPROACH_Z_SPEED     "FirstApproachMicronsPerSec"
#define FL_ROTATION             "FirstRotationMilliDegrees"
#define FL_EXPOSURE_WAIT        "FirstExposureWaitMS"
#define FL_SEPARATION_WAIT      "FirstSeparationWaitMS"
#define FL_APPROACH_WAIT        "FirstApproachWaitMS"
#define FL_PRESS                "FirstPressMicrons"
#define FL_PRESS_SPEED          "FirstPressMicronsPerSec"
#define FL_PRESS_WAIT           "FirstPressWaitMS"
#define FL_UNPRESS_SPEED        "FirstUnPressMicronsPerSec"

#define BI_SEPARATION_R_JERK    "BurnInSeparationRotJerk"
#define BI_SEPARATION_R_SPEED   "BurnInSeparationRPM"
#define BI_APPROACH_R_JERK      "BurnInApproachRotJerk"
#define BI_APPROACH_R_SPEED     "BurnInApproachRPM"
#define BI_Z_LIFT               "BurnInZLiftMicrons"
#define BI_SEPARATION_Z_JERK    "BurnInSeparationZJerk"
#define BI_SEPARATION_Z_SPEED   "BurnInSeparationMicronsPerSec"
#define BI_APPROACH_Z_JERK      "BurnInApproachZJerk"
#define BI_APPROACH_Z_SPEED     "BurnInApproachMicronsPerSec"
#define BI_ROTATION             "BurnInRotationMilliDegrees"
#define BI_EXPOSURE_WAIT        "BurnInExposureWaitMS"
#define BI_SEPARATION_WAIT      "BurnInSeparationWaitMS"
#define BI_APPROACH_WAIT        "BurnInApproachWaitMS"
#define BI_PRESS                "BurnInPressMicrons"
#define BI_PRESS_SPEED          "BurnInPressMicronsPerSec"
#define BI_PRESS_WAIT           "BurnInPressWaitMS"
#define BI_UNPRESS_SPEED        "BurnInUnPressMicronsPerSec"

#define ML_SEPARATION_R_JERK    "ModelSeparationRotJerk"
#define ML_SEPARATION_R_SPEED   "ModelSeparationRPM"
#define ML_APPROACH_R_JERK      "ModelApproachRotJerk"
#define ML_APPROACH_R_SPEED     "ModelApproachRPM"
#define ML_Z_LIFT               "ModelZLiftMicrons"
#define ML_SEPARATION_Z_JERK    "ModelSeparationZJerk"
#define ML_SEPARATION_Z_SPEED   "ModelSeparationMicronsPerSec"
#define ML_APPROACH_Z_JERK      "ModelApproachZJerk"
#define ML_APPROACH_Z_SPEED     "ModelApproachMicronsPerSec"
#define ML_ROTATION             "ModelRotationMilliDegrees"
#define ML_EXPOSURE_WAIT        "ModelExposureWaitMS"
#define ML_SEPARATION_WAIT      "ModelSeparationWaitMS"
#define ML_APPROACH_WAIT        "ModelApproachWaitMS"
#define ML_PRESS                "ModelPressMicrons"
#define ML_PRESS_SPEED          "ModelPressMicronsPerSec"
#define ML_PRESS_WAIT           "ModelPressWaitMS"
#define ML_UNPRESS_SPEED        "ModelUnPressMicronsPerSec"

// settings for pause & inspect
#define INSPECTION_HEIGHT       "InspectionHeightMicrons"
#define MAX_Z_TRAVEL            "MaxZTravelMicrons"

// settings for initializing motor controller
#define MICRO_STEPS_MODE        "MicroStepsMode"

#define Z_STEP_ANGLE            "ZStepAngleMillidegrees"
#define Z_MICRONS_PER_REV       "ZMicronsPerMotorRev"

#define R_STEP_ANGLE            "RStepAngleMillidegrees"
#define R_MILLIDEGREES_PER_REV  "RMilliDegreesPerMotorRev"

// motor control settings for homing
#define Z_HOMING_JERK           "ZHomingJerk" 
#define Z_HOMING_SPEED          "ZHomingSpeedMicronsPerSec" 
#define R_HOMING_JERK           "RHomingJerk" 
#define R_HOMING_SPEED          "RHomingSpeedRPM" 
#define R_HOMING_ANGLE          "RHomingAngleMilliDegrees" 

// motor control settings for starting a print/calibrating
#define Z_START_PRINT_JERK      "ZStartPrintJerk" 
#define Z_START_PRINT_SPEED     "ZStartPrintSpeedMicronsPerSec" 
#define Z_START_PRINT_POSITION  "ZStartPositionMicrons"
#define R_START_PRINT_JERK      "RStartPrintJerk" 
#define R_START_PRINT_SPEED     "RStartPrintSpeedRPM" 
#define R_START_PRINT_ANGLE     "RStartPrintPositionMillidegrees" 

#define HOME_ON_APPROACH        "RotateHomeOnApproach"

// The class that handles configuration and print options
class Settings 
{
public:
    Settings(std::string path);
    virtual ~Settings();
    bool Load(const std::string &filename, bool initializing = false);
    void Save(const std::string &filename);
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
    bool SetFromJSONString(const std::string &str);
    bool SetFromFile(const std::string &filename);
    
protected:
    std::string _settingsPath;
    std::set<std::string> _names;
    IErrorHandler* _errorHandler;
    bool IsValidSettingName(const std::string key);
    void EnsureSettingsDirectoryExists();
    bool AreSameType(Value& a, Value& b);
    Document _settingsDoc;
    const char* _defaults;  
};

// Singleton for sharing settings among all components
class PrinterSettings : public Settings
{
public:
    static Settings& Instance();
    
private:
    PrinterSettings();
    PrinterSettings(std::string path);
    PrinterSettings(PrinterSettings const&);
    PrinterSettings& operator=(PrinterSettings const&);
    ~PrinterSettings();
};
#endif    // SETTINGS_H

