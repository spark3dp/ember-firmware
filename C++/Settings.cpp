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
_settingsPath(path),
_errorHandler(&LOGGER)
{
    // create map of default values
    _defaultsMap["JobName"] = "slice";
    _defaultsMap["LayerThicknessMicrons"] = "25";
    _defaultsMap["BurnInLayers"] = "5";
    _defaultsMap["FirstExposureSec"] = "5.0";
    _defaultsMap["BurnInExposureSec"] = "2.0";
    _defaultsMap["ModelExposureSec"] = "1.5";
    _defaultsMap["IsRegistered"] = "false";
    
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

/// Restore all Settings to their default values
void Settings::RestoreAll()
{
    try
    {
        _settingsTree.clear();
        
        // restore from the map of default values
        std::map<std::string, std::string>::iterator it;
        for (it =_defaultsMap.begin(); it != _defaultsMap.end(); ++it)
        {
            _settingsTree.put("Settings." + it->first, it->second);
        }

        write_json(_settingsPath, _settingsTree);     
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_RESTORE_SETTINGS_FILE, true,   
                                                         _settingsPath.c_str());
    }
}

/// Restore a particular settings to its default value
void Settings::Restore(const std::string key)
{
    std::map<std::string, std::string>::iterator it = _defaultsMap.find(key);
    if(it != _defaultsMap.end())
        Set(key, _defaultsMap[key]);
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

/// Set  a new value for a saving and persist the change
void Settings::Set(const std::string key, const std::string value)
{
    try
    {
        _settingsTree.put("Settings." + key, value);
    }
    catch(ptree_error&)
    {
        _errorHandler->HandleError(CANT_SET_SETTING, true, key.c_str());
    }    
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
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
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
        _errorHandler->HandleError(CANT_GET_SETTING, true, key);
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
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
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
        _errorHandler->HandleError(CANT_GET_SETTING, true, key.c_str());
    }    
}

/// Gets the PrinterSettings singleton
Settings& PrinterSettings::Instance()
{
    static Settings settings(SETTINGS_PATH);

    return settings;
}

