/* 
 * File:   SettingsUT.cpp
 * Author: greener
 *
 * Created on Jun 17, 2014, 12:26:26 PM
 */

#include <stdlib.h>
#include <iostream>
#include <stdio.h>

#include <Settings.h>

/*
 * Simple C++ Test Suite
 */

void VerifyDefaults(Settings settings)
{
    if(settings.GetJobName().compare("") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default job name: " 
                << settings.GetJobName() << std::endl;
    }
    if(settings.GetLayerThicknessMicrons() != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default layer thickness: " 
                << settings.GetLayerThicknessMicrons() << std::endl;
    }
    if(settings.GetModelExposureTimeSec() != 1.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default model exposure time: " 
                << settings.GetModelExposureTimeSec() << std::endl;
    }
    if(settings.GetIsRegistered() != false)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default isRegistered:  true " 
                 << std::endl;
    }        
}

void VerifyModSettings(Settings settings)
{
    if(settings.GetJobName().compare("WhosYerDaddy") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new job name: " 
                << settings.GetJobName() << std::endl;
    }
    if(settings.GetLayerThicknessMicrons() != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new layer thickness: " 
                << settings.GetLayerThicknessMicrons() << std::endl;
    }
    if(settings.GetModelExposureTimeSec() != 3.14)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new model exposure time: " 
                << settings.GetModelExposureTimeSec() << std::endl;
    }
    if(settings.GetIsRegistered() != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new isRegistered: false" 
                  << std::endl;
    }    
}

#define TEST_SETTINGS_PATH ("/tmp/SettingsUT")
#define TEMP_PATH ("/tmp/MySettings")

void test1() {
    std::cout << "SettingsUT test 1" << std::endl;
    
    if (access(TEST_SETTINGS_PATH, F_OK) != -1) 
    {
        // settings file exists already, so delete it
        remove(TEST_SETTINGS_PATH);
    }
    
    if (access(TEMP_PATH, F_OK) != -1) 
    {
        // settings file exists already, so delete it
        remove(TEMP_PATH);
    }
    
  
    Settings settings(TEST_SETTINGS_PATH);
    
    // verify default values
    VerifyDefaults(settings);
    
    // set some setting values
    settings.Set("JobName", "WhosYerDaddy");
    settings.Set("LayerThicknessMicrons", "42");
    settings.Set("ModelExposureTimeSec", "3.14");
    settings.Set("IsRegistered", "true");
    
    VerifyModSettings(settings);    
    
    // save it to a  different file
    settings.Save(TEMP_PATH);
    
    // restore and verify default values
    settings.RestoreAll();
    VerifyDefaults(settings);
    
    // load new settings from the file and verify expected values
    settings.Load(TEMP_PATH);   
    VerifyModSettings(settings); 
    
   // TODO
   // Settings t2;
   // try loading a file that doesn't exist
   // t2.Load("UTSettings.json");
    
    // validate that settings have their default values
    // t2.GetJobName() ...
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% SettingsUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (SettingsUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (SettingsUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

