/* 
 * File:   Settings.cpp
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of Settings.
 *  
 * Created on June 4, 2014, 12:34 PM
 */

#include <string.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include <Settings.h>
#include <Logger.h>
#include <Filenames.h>

#define JOB_NAME_DEFAULT ("")
#define LAYER_THICKNESS_DEFAULT (25)
#define BURN_IN_LAYERS_DEFAULT (5)
#define FIRST_EXPOSURE_DEFAULT (5.0)
#define BURN_IN_EXPOSURE_DEFAULT (2.0)
#define MODEL_EXPOSURE_DEFAULT (1.5)
#define IS_REGISTERED_DEFAULT (false)

using boost::property_tree::ptree;
using boost::property_tree::ptree_error;

template<class Ptree>
inline const Ptree &empty_ptree()
{
    static Ptree pt;
    return pt;
}


/// Constructor.
Settings::Settings(std::string path) :
_settingsPath(path)
{
    try
    {
        Load(path);
    }
    catch(ptree_error&)
    {
        RestoreAll();
    }   
}

/// Destructor.
Settings::~Settings() {
}

/// Load all the Settings from a file
void Settings::Load(const std::string &filename)
{
    read_json(filename, _settingsTree);    
}

/// Save the current settings in the main settings file
void Settings::Save()
{
    write_json(_settingsPath, _settingsTree); 
}

/// Save all Settings in a file
void Settings::Save(const std::string &filename)
{
    write_json(filename, _settingsTree);    
}

/// Restore all Settings to their default values
void Settings::RestoreAll()
{
    _settingsTree.put("Settings.JobName", JOB_NAME_DEFAULT);
    _settingsTree.put("Settings.LayerThicknessMicrons", LAYER_THICKNESS_DEFAULT);
    _settingsTree.put("Settings.BurnInLayers", BURN_IN_LAYERS_DEFAULT);
    _settingsTree.put("Settings.FirstExposure", FIRST_EXPOSURE_DEFAULT);
    _settingsTree.put("Settings.BurnInExposure", BURN_IN_EXPOSURE_DEFAULT);
    _settingsTree.put("Settings.ModelExposureTimeSec", MODEL_EXPOSURE_DEFAULT);
    _settingsTree.put("Settings.IsRegistered", IS_REGISTERED_DEFAULT);

    write_json(_settingsPath, _settingsTree);       
}

/// Reload the settings from the settings file
void Settings::Refresh()
{
    Load(_settingsPath); 
}

/// Set  a new value for a saving and persist the change
void Settings::Set(const std::string key, const std::string value)
{
    _settingsTree.put("Settings." + key, value);
    Save();
}


// TODO: all setting names should be insensitive to case

/// Return the value of an integer setting.
int Settings::GetInt(const std::string key)
{
    try
    {
        return _settingsTree.get<int>("Settings." + key);
    }
    catch(ptree_error&)
    {
        // TODO: will need to do more than just logging this
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, key.c_str());
   }    
}

/// Returns the value of a string setting.
std::string Settings::GetString(const char* key)
{
    try
    {
        return _settingsTree.get<std::string>("Settings." + std::string(key));
    }
    catch(ptree_error&)
    {
        // TODO: will need to do more than just logging this
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, key);
    }    
}

/// Returns the value of a double-precision floating point setting.
double Settings::GetDouble(const std::string key)
{
    try
    {
        return _settingsTree.get<double>("Settings." + key);
    }
    catch(ptree_error&)
    {
        // TODO: will need to do more than just logging this
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, key.c_str());
    }    
}

/// Returns the value of a boolean setting.
bool Settings::GetBool(const std::string key)
{
    try
    {
        return _settingsTree.get<bool>("Settings." + key);
    }
    catch(ptree_error&)
    {
        // TODO: will need to do more than just logging this
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, key.c_str());
    }    
}

/// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);

    return settings;
}

