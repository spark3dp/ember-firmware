/* 
 * File:   PrintDataUT.cpp
 * Author: Jason Lefley
 *
 * Created on Jun 27, 2014, 1:55:50 PM
 */

#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>
#include <Shared.h>
#include <LayerSettings.h>

int mainReturnValue = EXIT_SUCCESS;

std::string testStagingDir, testDownloadDir, testPrintDataDir, testSettingsDir;

void Setup()
{
    testStagingDir = CreateTempDir();
    testDownloadDir = CreateTempDir();
    testPrintDataDir = CreateTempDir();
    testSettingsDir = CreateTempDir();
    
    SETTINGS.Set(STAGING_DIR, testStagingDir);
    SETTINGS.Set(DOWNLOAD_DIR, testDownloadDir);
    SETTINGS.Set(PRINT_DATA_DIR, testPrintDataDir);
}

void TearDown()
{
    SETTINGS.Restore(STAGING_DIR);
    SETTINGS.Restore(DOWNLOAD_DIR);
    SETTINGS.Restore(PRINT_DATA_DIR);
    
    RemoveDir(testStagingDir);
    RemoveDir(testDownloadDir);
    RemoveDir(testPrintDataDir);
    RemoveDir(testSettingsDir);
    
    testStagingDir = "";
    testDownloadDir = "";
    testPrintDataDir = "";
    testSettingsDir = "";
}

/// Place an print file archive in the download directory and stage it
void Stage(std::string filePath)
{
    Copy(filePath, testDownloadDir);
    PrintData printData;
    printData.Stage();
}

void ValidateTest()
{
    std::cout << "PrintDataUT validate test" << std::endl;
    
    PrintData printData;
   
    // Staging folder containing no images fails validation
    if (printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain any images, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Staging folder not containing first slice fails validation
    Touch(testStagingDir + "/slice_2.png");
    if (printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain first slice image, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    PurgeDirectory(testStagingDir);
    Touch(testStagingDir + "/slice_1.png");
    if (!printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return true when staging folder contains first slice image, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Staging folder containing slices with naming gap fails validation
    Touch(testStagingDir + "/slice_3.png");
    if (printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder contains slices with a naming gap, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    ;
    Touch(testStagingDir + "/slice_2.png");
    if (!printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return true when staging folder contains 3 slice images, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Staging folder containing a slice 0 fails validation
    Touch(testStagingDir + "/slice_0.png");
    if (printData.Validate(testStagingDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder contains slice 0 image, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void StageTest() {
    std::cout << "PrintDataUT stage test" << std::endl;
    
    // Place an print file archive in the download directory
    std::string printFile = testDownloadDir + "/print.tar.gz";
    Copy("resources/print.tar.gz", printFile);
    
    // Place a stray file in the staging directory
    std::string stray_file = testStagingDir + "/stray_file";
    Touch(stray_file);
    
    std::string slice1 = testStagingDir + "/slice_1.png";
    std::string slice2 = testStagingDir + "/slice_2.png";
    
    PrintData printData;
    
    bool success = printData.Stage();
    
    // Staging directory is cleared
    if (std::ifstream(stray_file.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to empty staging directory, stray file still in staging directory" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Print file is extracted to staging directory
    if (!(std::ifstream(slice1.c_str()) && std::ifstream(slice2.c_str())))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to extract print job archive to staging directory, did not find extracted files" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Print file is deleted after successful extraction
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to remove extracted print file, but print file is still in download directory" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Job name is stored
    std::string jn = printData.GetFileName();
    if (jn != "print.tar.gz")
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to store print file name as print.tar.gz, got \"" << jn << "\"" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Stage returns true if all operations are successful
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
            "message=Expected Stage to return true if no errors, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Stage returns false if no print file exists in download directory
    PurgeDirectory(testDownloadDir);
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to return false when download directory does not contain any print file archives, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Stage returns false if archive is not a valid .tar.gz
    Copy("resources/invalid.tar.gz", testDownloadDir);
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to return false when download directory contains invalid archive, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void LoadSettingsTest()
{
    std::cout << "PrintDataUT load settings test" << std::endl;
    
    Stage("resources/print.tar.gz");
    
    PrintData printData;
    bool success = printData.LoadSettings();
    
    // Settings are loaded from settings file in print file archive
    int layerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
    if (layerThickness != 10)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to load settings from settings file (LayerThickness == 10), got (LayerThickness == " <<
                layerThickness << ")" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    std::string jobName = SETTINGS.GetString(JOB_NAME_SETTING);
    if (jobName != "MyPrintJob")
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to load settings from settings file (JobName == MyPrintJob), got (JobName == " <<
                jobName << ")" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns true on success
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return true if no errors, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns false if settings file does not exist in the staging area,
    // nor in a file downloaded from the web (at PRINT_SETTINGS_FILE))
    PurgeDirectory(testStagingDir);
    if (printData.LoadSettings())
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if settings file does not exist, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns false if settings cannot be loaded
    Stage("resources/print_with_invalid_settings.tar.gz");
    if (printData.LoadSettings())
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if settings file cannot be loaded, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns true if settings can be loaded from file downloaded from web
    system("cp resources/good_settings " TEMP_PRINT_SETTINGS_FILE);
    if (!printData.LoadSettings())
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return true if settings file downloaded from web can be loaded, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
     system("rm " TEMP_PRINT_SETTINGS_FILE);
     
    // test overload that takes a filename
    system((std::string("cp resources/good_settings ") + testSettingsDir).c_str());
    system((std::string("cp resources/bad_settings ")  + testSettingsDir).c_str());

    if(printData.LoadSettings(testSettingsDir + std::string("/bogus")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if required settings file doesn't exist, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    if(printData.LoadSettings(testSettingsDir + std::string("/bad_settings")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if settings file is invalid, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    if(!printData.LoadSettings(testSettingsDir + std::string("/good_settings")))
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return true if settings file is valid, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    jobName = SETTINGS.GetString(JOB_NAME_SETTING);
    if (jobName != "NewJobName")
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to load settings from settings file (JobName == NewJobName), got (JobName == " <<
                jobName << ")" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void MovePrintDataTest()
{
    Stage("resources/print.tar.gz");
    
    PrintData printData;
    bool success = printData.MovePrintData();
    
    std::string pdSlice1 = testPrintDataDir + "/slice_1.png";
    std::string pdSlice2 = testPrintDataDir + "/slice_2.png";
    
    // Copies slices to print data directory
    if (!(std::ifstream(pdSlice1.c_str()) && std::ifstream(pdSlice2.c_str())))
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData to copy slices to print data directory, files were not copied" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    std::string sSlice1 = testStagingDir + "/slice_1.png";
    std::string sSlice2 = testStagingDir + "/slice_2.png";
    
    // Clears staging directory
    if (std::ifstream(sSlice1.c_str()) || std::ifstream(sSlice2.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData to clear staging directory, staging directory not empty" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Returns true on success
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData to return true on success, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns false if unable to copy slices
    Stage("resources/print.tar.gz");
    SETTINGS.Set(PRINT_DATA_DIR, "/bogus/directory");
    if (printData.MovePrintData())
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData return false when unable to copy slices, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // Returns false if unable to clear staging directory
    SETTINGS.Set(STAGING_DIR, "bogus");
    if (printData.MovePrintData())
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData return false when unable to clear staging directory, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
}

void LayerParamsTest()
{
    Copy("resources/good_layer_params.csv", testPrintDataDir);
    std::string filename = testPrintDataDir;
    filename.append("/good_layer_params.csv");
    
    LayerSettings layerSettings;
    bool success = layerSettings.Load(filename);
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
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
            std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
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
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
        "message=Got unexpected value for layer " << badLayer << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    for(int i = 0, j = 101; i < 11; i++, j++)
    {
        double exp = layerSettings.GetDouble(j, MODEL_EXPOSURE);
        if(fabs(exp - (6.4 + 0.1 * i)) > epsilon)
        {
            std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
            "message=Got unexpected exposure "<< exp << " for layer " << j << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        int thick = layerSettings.GetInt(j, LAYER_THICKNESS);
        if(thick != 20 + 5 * i)
        {
            std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
            "message=Got unexpected thickness " << thick << " for layer " << j << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;            
        }
    }
    
    // try non-existent LayerParams file    
    success = !layerSettings.Load("nonesuch.csv");
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
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
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
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
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
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
        std::cout << "%TEST_FAILED% time=0 testname=LayerParamsTest (PrintDataUT) " <<
            "message=Expected Load to return false for file with duplicate column headings, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintDataUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% ValidateTest (PrintDataUT)" << std::endl;
    Setup();
    ValidateTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 ValidateTest (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% StageTest (PrintDataUT)" << std::endl;
    Setup();
    StageTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 StageTest (PrintDataUT)" << std::endl;
    
    std::cout << "%TEST_STARTED% LoadSettingsTest (PrintDataUT)" << std::endl;
    Setup();
    LoadSettingsTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 LoadSettingsTest (PrintDataUT)" << std::endl;
    
    std::cout << "%TEST_STARTED% MovePrintDataTest (PrintDataUT)" << std::endl;
    Setup();
    MovePrintDataTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 MovePrintDataTest (PrintDataUT)" << std::endl;
    
    std::cout << "%TEST_STARTED% LayerParamsTest (PrintDataUT)" << std::endl;
    Setup();
    LayerParamsTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 LayerParamsTest (PrintDataUT)" << std::endl;
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

