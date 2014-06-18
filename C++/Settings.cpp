/* 
 * File:   Settings.cpp
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of settings.
 *  
 * Created on June 4, 2014, 12:34 PM
 */

#include <string.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Settings.h>
#include <Logger.h>

using boost::property_tree::ptree;

template<class Ptree>
inline const Ptree &empty_ptree()
{
    static Ptree pt;
    return pt;
}


/// Constructor.
Settings::Settings() :
_jobName(""),
_layerThicknessMicrons(25),
_modelExposureTimeSec(2.5),
_isRegistered(false)
{
        
}

/// Destructor.
Settings::~Settings() {
}

/// Load all the settings from a file
void Settings::Load(const std::string &filename)
{
    ptree pt;
    read_json(filename, pt);    
    const ptree &settings = pt.get_child("settings", empty_ptree<ptree>());
    
    _jobName = pt.get<std::string>("settings.JobName", "");
    _layerThicknessMicrons = pt.get("settings.LayerThicknessMicrons", 25);
    _modelExposureTimeSec = pt.get("settings.ModelExposureTimeSec", 1.5);
    _isRegistered = pt.get("settings.IsRegistered", false);
    
}

/// Save all settings in a file
void Settings::Save(const std::string &filename)
{
   ptree pt;

   pt.put("settings.JobName", _jobName);
   pt.put("settings.LayerThicknessMicrons", _layerThicknessMicrons);
   pt.put("settings.ModelExposureTimeSec", _modelExposureTimeSec);
   pt.put("settings.IsRegistered", _isRegistered);

   write_json(filename, pt);    
}


// TODO: all setting names should be insensitive to case

/// Stub for method that returns the value of an integer setting.
int Settings::GetInt(const char* name)
{
    int retVal = 0;
    
    if(!strcmp(name, "BurnInLayers"))
        retVal = 5; 
    else if(!strcmp(name, "LayerThicknessMicons"))
        retVal = 25; 
    else
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, name);
     
     return retVal;
}

/// Stub for method that returns the value of a string setting.
const char* Settings::GetString(const char* name)
{
    const char* retVal = "";
    
    if(!strcmp(name, "JobName"))
            retVal = "test";
    else
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, name);

    return retVal;
}

/// Stub for method that returns the value of a double-precision floating point 
/// setting.
double Settings::GetDouble(const char* name)
{
    double retVal = 0.0;
    
    if(!strcmp(name, "FirstExposure"))
        retVal = 5.0;
    else if(!strcmp(name, "BurnInExposure"))
        retVal = 3.0;
    else if(!strcmp(name, "ModelExposure"))
        retVal = 2; 
    else
        Logger::LogError(LOG_WARNING, errno, UNKNOWN_SETTING, name);
    
    return retVal;
}
