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

void VerifyDefaults(Settings& settings)
{
    if(settings.GetString("JobName").compare("slice") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default job name: " 
                << settings.GetString("JobName") << std::endl;
    }
    if(settings.GetInt("LayerThicknessMicrons") != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default layer thickness: " 
                << settings.GetInt("LayerThicknessMicrons") << std::endl;
    }
    if(settings.GetDouble("ModelExposureSec") != 1.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default model exposure time: " 
                << settings.GetDouble("ModelExposureSec") << std::endl;
    }
    if(settings.GetBool("IsRegistered") != false)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default isRegistered:  true " 
                 << std::endl;
    }        
}

void VerifyModSettings(Settings& settings)
{
    if(settings.GetString("JobName").compare("WhosYerDaddy") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new job name: " 
                << settings.GetString("JobName") << std::endl;
    }
    if(settings.GetInt("LayerThicknessMicrons") != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new layer thickness: " 
                << settings.GetInt("LayerThicknessMicrons") << std::endl;
    }
    if(settings.GetDouble("ModelExposureSec") != 3.14)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new model exposure time: " 
                << settings.GetDouble("ModelExposureSec")  << std::endl;
    }
    if(settings.GetBool("IsRegistered") != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new isRegistered: false" 
                  << std::endl;
    }    
}

#define TEST_SETTINGS_PATH ("/tmp/SettingsUT")
#define TEMP_PATH ("/tmp/MySettings")

void test1() {
    std::cout << "SettingsUT test 1" << std::endl;
    
    // delete test settings files if they exist
    if (access(TEST_SETTINGS_PATH, F_OK) != -1) 
        remove(TEST_SETTINGS_PATH);
    
    if (access(TEMP_PATH, F_OK) != -1) 
        remove(TEMP_PATH);
  
    Settings settings(TEST_SETTINGS_PATH);
    
    // verify default values
    VerifyDefaults(settings);
    
    // set some setting values
    settings.Set("JobName", "WhosYerDaddy");
    settings.Set("LayerThicknessMicrons", "42");
    settings.Set("ModelExposureSec", "3.14");
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
    
    // check restore of individual settings
    settings.Restore("JobName");
    if(settings.GetString("JobName").compare("slice") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default job name: " 
                << settings.GetString("JobName") << std::endl;
    }
    settings.Restore("ModelExposureSec");
    if(settings.GetDouble("ModelExposureSec") != 1.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default model exposure time: " 
                << settings.GetDouble("ModelExposureSec")  << std::endl;
    }
    // make sure other settings not restored
    if(settings.GetInt("LayerThicknessMicrons") != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored layer thickness: " 
                << settings.GetInt("LayerThicknessMicrons") << std::endl;
    }
    if(settings.GetBool("IsRegistered") != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored isRegistered: false" 
                  << std::endl;
    }    
    
    // verify JSON string IO, by getting the string that has layer thickness 42
    std::string json = settings.GetAllSettingsAsJSONString();
    // restore defaults
    settings.RestoreAll();
    VerifyDefaults(settings);
    // now load from string with thickness = 42
    settings.LoadFromJSONString(json);
    // and verify that it was changed
    if(settings.GetInt("LayerThicknessMicrons") != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=JSON IO didn't restore layer thickness: " 
                << settings.GetInt("LayerThicknessMicrons") << std::endl;
    }

    // verify we don't change when JSON string refers to an unknown setting
    settings.RestoreAll();
    VerifyDefaults(settings);
    int pos = json.find("LayerThicknessMicrons");
    json.replace(pos, 5, "Later");
    settings.LoadFromJSONString(json);
    if(settings.GetInt("LayerThicknessMicrons") != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=improperly changed layer thickness: " 
                << settings.GetInt("LayerThicknessMicrons") << std::endl;
    }
    
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

