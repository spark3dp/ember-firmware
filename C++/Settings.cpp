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
#include <boost/foreach.hpp>

#include <Settings.h>
#include <Logger.h>
#include <Filenames.h>

#define ROOT "Settings"
#define ROOT_DOT ROOT "."

using boost::property_tree::ptree;
using boost::property_tree::ptree_error;

/// Constructor.
Settings::Settings(std::string path) :
_settingsPath(path),
_errorHandler(&LOGGER)
{
    // create map of default values
    _defaultsMap[JOB_NAME_SETTING] = "slice";
    _defaultsMap[LAYER_THICKNESS] = "25";
    _defaultsMap[BURN_IN_LAYERS] = "5";
    _defaultsMap[FIRST_EXPOSURE] = "5.0";
    _defaultsMap[BURN_IN_EXPOSURE] = "2.0";
    _defaultsMap[MODEL_EXPOSURE] = "1.5";
    _defaultsMap[SEPARATION_RPM] = "0";
    _defaultsMap[IS_REGISTERED] = "false";
    _defaultsMap[PRINT_DATA_DIR] = std::string(ROOT_DIR) +  "/print_data";
    _defaultsMap[DOWNLOAD_DIR] = std::string(ROOT_DIR) + "/download";
    _defaultsMap[STAGING_DIR] = std::string(ROOT_DIR) + "/staging";
    
    try
    {
        read_json(path, _settingsTree); 
    }
    catch(ptree_error&)
    {
        RestoreAll();
    }   
}

/// Destructor.
Settings::~Settings() 
{
}

/// Load all the Settings from a file
void Settings::Load(const std::string &filename)
{
    try
    {
        read_json(filename, _settingsTree);
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_LOAD_SETTINGS_FILE, true,  
                                                             filename.c_str());
    }
}

/// Load all the Settings from a string
bool Settings::LoadFromJSONString(const std::string &str)
{
    bool retVal = false;
    std::stringstream ss(str);
    
    try
    {
        ptree pt;
        read_json(ss, pt);
        
        // for each setting name from the given string
        BOOST_FOREACH(ptree::value_type &v, pt.get_child(ROOT))
        {
            if(IsValidSettingName(v.first))
            {
                // get its value from pt and set it into _settingsTree
                Set(v.first, v.second.data());
            }
        }
        Save();
        retVal = true;
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_READ_SETTINGS_FROM_STRING, true,  
                                                             str.c_str());
    }
    return retVal;
}


/// Save the current settings in the main settings file
void Settings::Save()
{
    Save(_settingsPath); 
}

/// Save the current settings in the given file
void Settings::Save(const std::string &filename)
{
    try
    {
        write_json(filename, _settingsTree);   
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_SAVE_SETTINGS_FILE, true,  
                                                             filename.c_str());
    }
}

/// Get all the settings a s a single text string in JSON.
std::string Settings::GetAllSettingsAsJSONString()
{
    std::stringstream ss;
    try
    {
        write_json(ss, _settingsTree);   
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_WRITE_SETTINGS_TO_STRING);
    }
    return ss.str();
}

/// Restore all Settings to their default values
void Settings::RestoreAll()
{
    try
    {
        _settingsTree.clear();
        
        // restore from the map of default values
        std::map<std::string, std::string>::iterator it;
        for (it = _defaultsMap.begin(); it != _defaultsMap.end(); ++it)
        {
            _settingsTree.put(ROOT_DOT + it->first, it->second);
        }

        write_json(_settingsPath, _settingsTree);     
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_RESTORE_SETTINGS_FILE, true,   
                                                         _settingsPath.c_str());
    }
}

/// Restore a particular setting to its default value
void Settings::Restore(const std::string key)
{
    if(IsValidSettingName(key))
    {
        Set(key, _defaultsMap[key]);
        Save();
    }
    else
    {
        _errorHandler->HandleError(NO_DEFAULT_FOR_SETTING, true, key.c_str());
    }
}

/// Reload the settings from the settings file
void Settings::Refresh()
{
    Load(_settingsPath); 
}

/// Set  a new value for a saving but don't persist the change
void Settings::Set(const std::string key, const std::string value)
{
    try
    {
        if(IsValidSettingName(key))
            _settingsTree.put(ROOT_DOT + key, value);
        else
            _errorHandler->HandleError(UNKNOWN_SETTING, true, key.c_str());
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_SET_SETTING, true, key.c_str());
    }    
}

/// Return the value of an integer setting.
int Settings::GetInt(const std::string key)
{
    int retVal = 0;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsTree.get<int>(ROOT_DOT + key);
        else
           _errorHandler->HandleError(UNKNOWN_SETTING, true, key.c_str()); 
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
    }  
    return retVal;
}

/// Returns the value of a string setting.
std::string Settings::GetString(const std::string key)
{
    std::string retVal("");
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsTree.get<std::string>(ROOT_DOT + key);
        else
           _errorHandler->HandleError(UNKNOWN_SETTING, true, key.c_str()); 
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
    }  
    return retVal;
}

/// Returns the value of a double-precision floating point setting.
double Settings::GetDouble(const std::string key)
{
    double retVal = 0.0;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsTree.get<double>(ROOT_DOT + key);
        else
           _errorHandler->HandleError(UNKNOWN_SETTING, true, key.c_str()); 
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
    } 
    return retVal;
}

/// Returns the value of a boolean setting.
bool Settings::GetBool(const std::string key)
{
    bool retVal = false;
    try
    {
        if(IsValidSettingName(key))
            retVal = _settingsTree.get<bool>(ROOT_DOT + key);
        else
           _errorHandler->HandleError(UNKNOWN_SETTING, true, key.c_str()); 
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
    }  
    return retVal;
}

/// Validates that a setting name is one for which we have a default value.
bool Settings::IsValidSettingName(const std::string key)
{
    std::map<std::string, std::string>::iterator it = _defaultsMap.find(key);
    return it != _defaultsMap.end();
}

/// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);

    return settings;
}

