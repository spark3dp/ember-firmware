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

/// Constructor.
Settings::Settings() {
}

/// Destructor.
Settings::~Settings() {
}

/// Stub for method that returns the value of an integer setting.
int Settings::GetInt(const char* name)
{
    
}

/// Stub for method that returns the value of a string setting.
const char* Settings::GetString(const char* name)
{
    const char* retVal = "";
    
    if(!strcmp(name, "JobName"))
            retVal = "test";
    
    return retVal;
}

/// Stub for method that returns the value of a double-precision floating point 
/// setting.
double Settings::GetDouble(const char* name)
{
    
}
