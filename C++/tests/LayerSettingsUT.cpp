/* 
 * File:   LayerSettingsUT.cpp
 * Author: Richard Greene
 *
 * Created on May 22, 2015, 6:43:11 PM
 */

#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <Settings.h>
#include <Shared.h>
#include <LayerSettings.h>

int mainReturnValue = EXIT_SUCCESS;

std::string testPrintDataDir, testSettingsDir;

void Setup()
{
    testPrintDataDir = CreateTempDir();
    testSettingsDir = CreateTempDir();
    
    SETTINGS.Set(PRINT_DATA_DIR, testPrintDataDir);
}

void TearDown()
{
    SETTINGS.Restore(PRINT_DATA_DIR);
    
    RemoveDir(testPrintDataDir);
    RemoveDir(testSettingsDir);
    
    testPrintDataDir = "";
    testSettingsDir = "";
}

void LayerSettingsTest()
{
    Copy("resources/good_layer_params.csv", testPrintDataDir);
    std::string filename = testPrintDataDir;
    filename.append("/good_layer_params.csv");
    
    LayerSettings layerSettings;
    bool success = layerSettings.Load(filename);
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Expected Load to return true on success, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // check that we get the expected values
    for(int i = -1; i < 11; i++)
        if(layerSettings.GetDouble(i, MODEL_EXPOSURE) != 
                                      SETTINGS.GetDouble(MODEL_EXPOSURE) ||
           layerSettings.GetInt(i, LAYER_THICKNESS) !=
                                      SETTINGS.GetInt(LAYER_THICKNESS))
        {
            std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Got unexpected override for layer " << i << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    
    int badLayer = -1;
    double epsilon = 0.001;
       
    if(fabs(layerSettings.GetDouble(11, MODEL_EXPOSURE) - 3.3) > epsilon ||
            layerSettings.GetInt(11, LAYER_THICKNESS) != 20)
        badLayer = 11;
    else if(fabs(layerSettings.GetDouble(12, MODEL_EXPOSURE) - 5.1) > epsilon ||
            layerSettings.GetInt(12, LAYER_THICKNESS) != 
                                               SETTINGS.GetInt(LAYER_THICKNESS))
        badLayer = 12;
    else if(layerSettings.GetDouble(13, MODEL_EXPOSURE) != 
                                          SETTINGS.GetDouble(MODEL_EXPOSURE) ||
            layerSettings.GetInt(13, LAYER_THICKNESS) != 15)
        badLayer = 13;
    else if(fabs(layerSettings.GetDouble(14, MODEL_EXPOSURE) - 6.1) > epsilon ||
            layerSettings.GetInt(14, LAYER_THICKNESS) != 5)
        badLayer = 14;
    else if(layerSettings.GetDouble(1005, MODEL_EXPOSURE) != 
                                          SETTINGS.GetDouble(MODEL_EXPOSURE) ||
            layerSettings.GetInt(1005, LAYER_THICKNESS) != 10)
        badLayer = 1005;
    
    if(badLayer > 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
        "message=Got unexpected value for layer " << badLayer << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    for(int i = 0, j = 101; i < 11; i++, j++)
    {
        double exp = layerSettings.GetDouble(j, MODEL_EXPOSURE);
        if(fabs(exp - (6.4 + 0.1 * i)) > epsilon)
        {
            std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Got unexpected exposure "<< exp << " for layer " << j << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        int thick = layerSettings.GetInt(j, LAYER_THICKNESS);
        if(thick != 20 + 5 * i)
        {
            std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Got unexpected thickness " << thick << " for layer " << j << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;            
        }
    }
    
    // try non-existent LayerParams file    
    success = !layerSettings.Load("nonesuch.csv");
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Expected Load to return false for nonexistent file, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // try "good_settings" file as a case of a bad LayerParams file
    Copy("resources/good_settings", testPrintDataDir);
    filename = testPrintDataDir;
    filename.append("/good_settings");
    
    success = !layerSettings.Load(filename);
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest) " <<
            "message=Expected Load to return false for invalid file, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // try file with more than one entry for layer 10
    Copy("resources/bad_layer_params.csv", testPrintDataDir);
    filename = testPrintDataDir;
    filename.append("/bad_layer_params.csv");
    
    success = !layerSettings.Load(filename);
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Expected Load to return false for file with duplicate entries, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // try file with more than one column for exposure times
    // in duplicate_columns_layer_params.csv
    Copy("resources/duplicate_columns_layer_params.csv", testPrintDataDir);
    filename = testPrintDataDir;
    filename.append("/duplicate_columns_layer_params.csv");
    
    success = !layerSettings.Load(filename);
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerSettingsTest " <<
            "message=Expected Load to return false for file with duplicate column headings, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}


int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% LayerSettingsUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (LayerSettingsUT)" << std::endl;
    Setup();
    LayerSettingsTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 test1 (LayerSettingsUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

