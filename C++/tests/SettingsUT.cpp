/* 
 * File:   SettingsUT.cpp
 * Author: greener
 *
 * Created on Jun 17, 2014, 12:26:26 PM
 */

#include <stdlib.h>
#include <iostream>

#include <Settings.h>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "SettingsUT test 1" << std::endl;
    
    Settings settings;
    
    // set some setting values
    settings.SetJobName("WhosYerDaddy");
    settings.SetLayerThicknessMicrons(42);
    settings.SetModelExposureTimeSec(3.14);
    settings.SetIsRegistered(true);
    
    // save it to a file
    settings.Save("/tmp/MySettings.json");
    
    // create a new instance of settings and verify initial default values
    Settings newSettings;
    if(newSettings.GetJobName().compare("") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default job name: " 
                << newSettings.GetJobName() << std::endl;
    }
    if(newSettings.GetLayerThicknessMicrons() != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default layer thickness: " 
                << newSettings.GetLayerThicknessMicrons() << std::endl;
    }
    if(newSettings.GetModelExposureTimeSec() != 2.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default model exposure time: " 
                << newSettings.GetModelExposureTimeSec() << std::endl;
    }
    if(newSettings.GetIsRegistered() != false)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default isRegistered:  true " 
                 << std::endl;
    }    
    
    // load new settings from the file and verify expected values
    newSettings.Load("/tmp/MySettings.json");   
    
    if(newSettings.GetJobName().compare("WhosYerDaddy") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new job name: " 
                << newSettings.GetJobName() << std::endl;
    }
    if(newSettings.GetLayerThicknessMicrons() != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new layer thickness: " 
                << newSettings.GetLayerThicknessMicrons() << std::endl;
    }
    if(newSettings.GetModelExposureTimeSec() != 3.14)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new model exposure time: " 
                << newSettings.GetModelExposureTimeSec() << std::endl;
    }
    if(newSettings.GetIsRegistered() != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new isRegistered: false" 
                  << std::endl;
    }
    
   
   // TODO
    Settings t2;
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

