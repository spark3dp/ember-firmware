/* 
 * File:   Settings.h
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of settings.
 * 
 * Created on June 4, 2014, 12:34 PM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

#include <string>
#include <set>

#include <rapidjson/document.h>

#include <Logger.h>

using namespace rapidjson;

#define SETTINGS (PrinterSettings::Instance())

/// setting name strings
#define JOB_NAME_SETTING "JobName"
#define LAYER_THICKNESS  "LayerThicknessMicrons"
#define BURN_IN_LAYERS   "BurnInLayers"
#define FIRST_EXPOSURE   "FirstExposureSec"
#define BURN_IN_EXPOSURE "BurnInExposureSec"
#define MODEL_EXPOSURE   "ModelExposureSec"
#define SEPARATION_RPM   "SeparationRPMOffset"
#define IS_DEVELOPER     "IsDeveloper"
#define PRINT_DATA_DIR   "PrintDataDir"
#define DOWNLOAD_DIR     "DownloadDir"
#define STAGING_DIR      "StagingDir"
#define HARDWARE_REV     "HardwareRev"

#define CALIBRATION_Z_TRAVEL    "CalibrationZTravelMicrons"
// FL = first layer, BI = burn-in layer, ML = model Layer
#define FL_EXPOSURE_WAIT        "FirstExposureWaitSec"
#define FL_SEPARATION_WAIT      "FirstSeparationWaitSec"
#define FL_APPROACH_WAIT        "FirstApproachWaitSec"

#define BI_EXPOSURE_WAIT        "BurnInExposureWaitSec"
#define BI_SEPARATION_WAIT      "BurnInSeparationWaitSec"
#define BI_APPROACH_WAIT        "BurnInApproachWaitSec"

#define ML_EXPOSURE_WAIT        "ModelExposureWaitSec"
#define ML_SEPARATION_WAIT      "ModelSeparationWaitSec"
#define ML_APPROACH_WAIT        "ModelApproachWaitSec"

#define FL_SEPARATION_R_SPEED   "FirstSeparationRPM"
#define FL_APPROACH_R_SPEED     "FirstApproachRPM"
#define FL_SEPARATION_Z_SPEED   "FirstSeparationMMPerSec"
#define FL_APPROACH_Z_SPEED     "FirstApproachMMPerSec"
#define FL_Z_LIFT               "FirstZLiftMM"
#define FL_ROTATION             "FirstRotationDegrees"

#define BI_SEPARATION_R_SPEED   "BurnInSeparationRPM"
#define BI_APPROACH_R_SPEED     "BurnInApproachRPM"
#define BI_SEPARATION_Z_SPEED   "BurnInSeparationMMPerSec"
#define BI_APPROACH_Z_SPEED     "BurnInApproachMMPerSec"
#define BI_Z_LIFT               "BurnInZLiftMM"
#define BI_ROTATION             "BurnInRotationDegrees"

#define ML_SEPARATION_R_SPEED   "ModelSeparationRPM"
#define ML_APPROACH_R_SPEED     "ModelApproachRPM"
#define ML_SEPARATION_Z_SPEED   "ModelSeparationMMPerSec"
#define ML_APPROACH_Z_SPEED     "ModelApproachMMPerSec"
#define ML_Z_LIFT               "ModelZLiftMM"
#define ML_ROTATION             "ModelRotationDegrees"


/// The class that handles configuration and print options
class Settings 
{
public:
    Settings(std::string path);
    virtual ~Settings();
    bool Load(const std::string &filename, bool ignoreErrors = false);
    void Save(const std::string &filename);
    void Save();
    void RestoreAll();
    void Restore(const std::string key);
    void Refresh();
    void Set(const std::string key, const std::string value);
    void Set(const std::string key, int value);
    void Set(const std::string key, double value);
    void Set(const std::string key, bool value);
    int GetInt(const std::string key);
    std::string GetString(const std::string key);
    double GetDouble(const std::string key);
    bool GetBool(const std::string key);
    void SetErrorHandler(IErrorHandler* handler) { _errorHandler = handler; }
    std::string GetAllSettingsAsJSONString();
    bool LoadFromJSONString(const std::string &str);
    
protected:
    // don't allow construction without supplying file name
    Settings();
    std::string _settingsPath;
    std::set<std::string> _names;
    IErrorHandler* _errorHandler;
    bool IsValidSettingName(const std::string key);
    void EnsureSettingsDirectoryExists();
    Document _settingsDoc;
    const char* _defaults;  
};

/// Singleton for sharing settings among all components
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
#endif	/* SETTINGS_H */

