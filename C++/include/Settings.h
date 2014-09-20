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
#define IS_REGISTERED    "IsRegistered"
#define PRINT_DATA_DIR   "PrintDataDir"
#define DOWNLOAD_DIR     "DownloadDir"
#define STAGING_DIR      "StagingDir"
#define R_HOMING_ACCEL   "RHomingAccelerationPct" 
#define R_HOMING_SPEED   "RHomingSpeedRPM" 
#define Z_HOMING_ACCEL   "ZHomingAccelerationPct" 
#define Z_HOMING_SPEED   "ZHomingSpeedMMPerSec" 
        
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

