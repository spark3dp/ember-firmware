/* 
 * File:   Settings.cpp
 * Author: Richard Greene
 *
 * Handles storage, retrieval, and reset of settings.
 *  
 * Created on June 4, 2014, 12:34 PM
 */

#include <string.h>

#include <Settings.h>
#include <Logger.h>

/// Constructor.
Settings::Settings() {
}

/// Destructor.
Settings::~Settings() {
}

// TODO: all setting names should be insensitive to case

/// Stub for method that returns the value of an integer setting.
int Settings::GetInt(const char* name)
{
    int retVal = 0;
    
    if(!strcmp(name, "BurnInLayers"))
        retVal = 5; 
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
