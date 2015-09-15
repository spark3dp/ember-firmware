//  File:   SettingsUT.cpp
//  Tests Settings
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>

#include <Settings.h>

int mainReturnValue = EXIT_SUCCESS;

void VerifyDefaults(Settings& settings)
{
    if (settings.GetString(JOB_NAME_SETTING).compare("") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    if (settings.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    double exp = settings.GetDouble(MODEL_EXPOSURE);
    if (exp > 2.51 || exp < 2.49)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default model exposure time: " 
                  << exp << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
}

void VerifyModSettings(Settings& settings)
{
    if (settings.GetString(JOB_NAME_SETTING).compare("WhosYerDaddy") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    if (settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    if (settings.GetDouble(MODEL_EXPOSURE) != 3.14)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new model exposure time: " 
                << settings.GetDouble(MODEL_EXPOSURE)  << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
}

bool gotError = false;

class ErrorHandler: public IErrorHandler
{
    void HandleError(ErrorCode, bool fatal, const char* str, int value)
    {
        gotError = true;
    }  
};

void VerifyExpectedError(const char* msg)
{
    if (!gotError)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=No error from " << msg
                  << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    else
    {
       std::cout << "Got expected error from " << msg << std::endl; 
    }
    gotError = false;
}

std::string tempDir;

void Setup()
{
    tempDir = CreateTempDir();
}

void TearDown()
{
    RemoveDir(tempDir);
}

void test1() {
    std::cout << "SettingsUT test 1" << std::endl;
    
    std::string tempPath = tempDir + "/MySettings";
    std::string testSettingsPath = tempDir + "/SettingsUT";
    std::string newPath = tempDir + "/newSettings";
    
    Settings settings(testSettingsPath);
    
    // for testing error conditions
    ErrorHandler eh;
    settings.SetErrorHandler(&eh);

    
    // verify default values
    VerifyDefaults(settings);
    
    // set some setting values
    settings.Set(JOB_NAME_SETTING, std::string("WhosYerDaddy"));   
    settings.Set(LAYER_THICKNESS, 42); 
    settings.Set(MODEL_EXPOSURE, 3.14);

    
    VerifyModSettings(settings);  
    
    // verify settings not yet persisted
    std::ifstream file(testSettingsPath.c_str());

    if (file)
    {
        std::stringstream buffer;

        buffer << file.rdbuf();

        file.close();

        if (buffer.str().find("WhosYerDaddy") != std::string::npos)
        {
            std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Settings persisted when they shouldn't be"
                  << std::endl;
            mainReturnValue = EXIT_FAILURE;
        }
    }
    
    
    // save it to a  different file
    settings.Save(tempPath);
    
    // restore and verify default values
    settings.RestoreAll();
    VerifyDefaults(settings);
    
    // load new settings from the file and verify expected values
    settings.Load(tempPath);   
    VerifyModSettings(settings); 
    // check restore of only print-specific settings, 
    // by changing a general printer setting
    settings.Set(HARDWARE_REV, 0);
    settings.RestoreAllPrintSettings();
    VerifyDefaults(settings);
    // make sure printer setting not restored
    if (settings.GetInt(HARDWARE_REV) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored hardware rev: " 
                << settings.GetInt(HARDWARE_REV) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // load new settings from the file and verify expected values again
    settings.Load(tempPath);   
    VerifyModSettings(settings); 
    
    // check restore of individual settings
    settings.Restore(JOB_NAME_SETTING);
    if (settings.GetString(JOB_NAME_SETTING).compare("") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    settings.Restore(MODEL_EXPOSURE);
    if (settings.GetDouble(MODEL_EXPOSURE) != 2.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default model exposure time: " 
                << settings.GetDouble(MODEL_EXPOSURE)  << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    // make sure other settings not restored
    if (settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
        
    // verify JSON string IO, by getting the string that has layer thickness 42
    std::string json = settings.GetAllSettingsAsJSONString();
    std::cout << json << std::endl;
    
    // restore defaults
    settings.RestoreAll();
    VerifyDefaults(settings);
    // now load from string with thickness = 42
    bool retVal = settings.SetFromJSONString(json);
    if (!retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Couldn't LoadFromJSONString" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    // and verify that it was changed
    if (settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=JSON IO didn't restore layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }

    // verify we don't change when JSON string refers to an unknown setting
    settings.RestoreAll();
    VerifyDefaults(settings);
    int pos = json.find(LAYER_THICKNESS);
    json.replace(pos, 5, "Later");
    gotError = false;
    retVal = settings.SetFromJSONString(json);
    VerifyExpectedError("unknown setting name in JSON string");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString returned true even though there was an unknown setting name" << std::endl;    
        mainReturnValue = EXIT_FAILURE;
    }
    if (settings.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=improperly changed layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // verify changes to the settings file only take effect after a refresh
    settings.RestoreAll();  // reset TEST_SETTINGS_PATH to its defaults
    Settings mod(tempPath);
    Copy(testSettingsPath, tempPath);
    if (mod.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=copying a file changed a setting" 
                 << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    mod.Refresh();
    if (mod.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Refresh failed to load changed setting" 
                 << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    // try loading a file that doesn't exist
    settings.RestoreAll();
    gotError = false;
    settings.Load("NonExistentFile");
    VerifyExpectedError("non-existent file");
    VerifyDefaults(settings);
    
    // try loading an improperly formatted file
    settings.Load("/smith/smith");
    VerifyExpectedError("improperly formatted file");
    VerifyDefaults(settings);
    
    // try loading files with settings of wrong type
    settings.Load("resources/wrong_type_settings_1");
    VerifyExpectedError("file with settings of wrong type");
    VerifyDefaults(settings);
    
    settings.Load("resources/wrong_type_settings_2");
    VerifyExpectedError("another file with settings of wrong type");
    VerifyDefaults(settings);
    
    // try reading from a non-JSON string
    retVal = settings.SetFromJSONString("This is clearly not a JSON settings string!");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString returned true when it should have failed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("improperly formatted  string");
    VerifyDefaults(settings);
    
    // try reading settings of the wrong type from a JSON string
    retVal = settings.SetFromJSONString("{\"Settings\":{\"LayerThicknessMicrons\":25.0}}");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have failed due to double where int expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("double where int expected");
    VerifyDefaults(settings);
    
    retVal = settings.SetFromJSONString("{\"Settings\":{\"LayerThicknessMicrons\":\"25\"}}");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have failed due to string where int expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("string where int expected");
    VerifyDefaults(settings);
    
    retVal = settings.SetFromJSONString("{\"Settings\":{\"JobName\":25}}");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have failed due to int where string expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("int where string expected");
    VerifyDefaults(settings);
    
    retVal = settings.SetFromJSONString("{\"Settings\":{\"JobName\":25.0}}");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have failed due to double where string expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("double where string expected");
    VerifyDefaults(settings);
        
    retVal = settings.SetFromJSONString("{\"Settings\":{\"ModelExposureSec\":\"25\"}}");
    if (retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have failed due to string where double expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("string where double expected");
    VerifyDefaults(settings);
   
    // attempt to set settings contained in file when specified file doesn't exist
    if (settings.SetFromFile(tempDir + std::string("/bogus")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) " <<
            "message=Expected SetFromFile to return false if required settings file doesn't exist, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyDefaults(settings);

    // attempt to set settings contained in file when specified file contains bad settings
    system((std::string("cp resources/bad_settings ")  + tempDir).c_str());

    if (settings.SetFromFile(tempDir + std::string("/bad_settings")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) " <<
            "message=Expected SetFromFile to return false if settings file is invalid, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    VerifyExpectedError("file with invalid settings");
    VerifyDefaults(settings);
    
    retVal = settings.SetFromJSONString("{\"Settings\":{\"ModelExposureSec\":25}}");
    if (!retVal)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString should have accepted int where double expected" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }

    // set settings contained in file when file exists and contains valid settings
    system((std::string("cp resources/good_settings ") + tempDir).c_str());

    if (!settings.SetFromFile(tempDir + std::string("/good_settings")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) " <<
            "message=Expected SetFromFile to return true if settings file is valid, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    
    std::string jobName = settings.GetString(JOB_NAME_SETTING);

    if (jobName != "NewJobName")
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) " <<
            "message=Expected SetFromFile to load settings from settings file (JobName == NewJobName), got (JobName == " <<
                jobName << ")" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    settings.RestoreAll();
    VerifyDefaults(settings);
    
    // attempt to save to an illegal file name
    settings.Save("badFilename*//?");
    VerifyExpectedError("saving to illegal filename");
    VerifyDefaults(settings);
    
    // attempt to save to file that's write protected
    // (for some reason, this doesn't prevent Save() from working)
//    system ("chmod 0000 " TEST_SETTINGS_PATH);
//    settings.Save();
//    VerifyExpectedError("saving to read-only file");
//    VerifyDefaults(settings);
    
    // attempt to restore a non-existent setting
    settings.Restore("ThisIsATest");
    VerifyExpectedError("restoring a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to set a non-existent setting
    settings.Set(std::string("Material"), std::string("PureUnobtanium"));
    VerifyExpectedError("setting a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent setting
    int x = settings.GetInt("XYZ");
    VerifyExpectedError("getting a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to get the wrong type of setting
    x = settings.GetInt(JOB_NAME_SETTING);
    VerifyExpectedError("getting string as int setting");
    VerifyDefaults(settings);
    
    // attempt to get the wrong type of setting
    double y = settings.GetDouble(JOB_NAME_SETTING);
    VerifyExpectedError("getting string as double setting");
    VerifyDefaults(settings);
      
  // no error here because any type of setting can be interpreted as a string!
     // attempt to get the wrong type of setting
//    std::string str = settings.GetString(LAYER_THICKNESS);
//    VerifyExpectedError("No error getting int as string setting");
//    VerifyDefaults(settings);

     // attempt to get a non-existent string setting
    std::string str = settings.GetString("WhosYerMama");
    VerifyExpectedError("getting non-existent string setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent int setting
    x = settings.GetInt("WhosYerMama");
    VerifyExpectedError("getting non-existent int setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent double setting
    y = settings.GetDouble("WhosYerMama");
    VerifyExpectedError("getting non-existent double setting");
    VerifyDefaults(settings);
    
    // verify that having newly defined settings doesn't lose existing values of 
    // any existing settings
    system((std::string("cp resources/good_settings ") + newPath).c_str());
    Settings newSettings(newPath);
    if (newSettings.GetInt(LAYER_THICKNESS) != 15 ||
       newSettings.GetString(JOB_NAME_SETTING) != "NewJobName")
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Old values lost when new settings added" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    if (newSettings.GetInt(BURN_IN_LAYERS) != 1)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=New settings not added" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% SettingsUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (SettingsUT)" << std::endl;
    Setup();
    test1();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 test1 (SettingsUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

