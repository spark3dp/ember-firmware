/* 
 * File:   Thermometer.cpp
 * Author: Richard Greene
 * 
 * Measures and reports temperature.
 * 
 * Created on October 22, 2014, 12:58 PM
 */
#include <iostream>
#include <glob.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>

#include <Thermometer.h>
#include <Settings.h>
#include <Logger.h>

/// Get the name of the file that holds temperature measurements.
Thermometer::Thermometer() 
{
    glob_t gl;

    if(glob("/mnt/1wire/22.*/temperature", GLOB_NOSORT, NULL, &gl) == 0)
    {
        _temperatureFile = gl.gl_pathv[0];
    }
    else
    {
        if(SETTINGS.GetInt(HARDWARE_REV) != 0)
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(CantOpenThermometer));
            exit(1);
        }
        _temperatureFile.clear();
    }
    
    globfree(&gl);    
}

/// Measure the current temperature and store it for future reference.  
double Thermometer::GetTemperature()
{
    if(_temperatureFile.empty())
        return -1.0;
    
    std::string line;
    std::ifstream infile(_temperatureFile.c_str());
    std::getline(infile, line);
    
    // convert to a double
    return strtod(line.c_str(), NULL);
}