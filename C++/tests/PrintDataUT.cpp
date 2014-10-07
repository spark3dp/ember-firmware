/* 
 * File:   PrintDataUT.cpp
 * Author: Jason Lefley
 *
 * Created on Jun 27, 2014, 1:55:50 PM
 */

#include <stdlib.h>
#include <iostream>

#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>

std::string testStagingDir, testDownloadDir, testPrintDataDir;

void Setup()
{
    testStagingDir = CreateTempDir();
    testDownloadDir = CreateTempDir();
    testPrintDataDir = CreateTempDir();
    
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
    
    testStagingDir = "";
    testDownloadDir = "";
    testPrintDataDir = "";
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
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain any images, got true" << std::endl;
        return;
    }
    
    // Staging folder not containing first slice fails validation
    Touch(testStagingDir + "/slice_2.png");
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain first slice image, got true" << std::endl;
        return;
    }
    
    PurgeDirectory(testStagingDir);
    Touch(testStagingDir + "/slice_1.png");
    if (!printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return true when staging folder contains first slice image, got false" << std::endl;
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
        return;
    }
    
    // Print file is extracted to staging directory
    if (!(std::ifstream(slice1.c_str()) && std::ifstream(slice2.c_str())))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to extract print job archive to staging directory, did not find extracted files" << std::endl;
        return;
    }
    
    // Print file is deleted after successful extraction
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to remove extracted print file, but print file is still in download directory" << std::endl;
        return;
    }
    
    // Job name is stored
    std::string jn = printData.GetFileName();
    if (jn != "print.tar.gz")
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to store print file name as print.tar.gz, got \"" << jn << "\"" << std::endl;
        return;
    }
    
    // Stage returns true if all operations are successful
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
            "message=Expected Stage to return true if no errors, got false" << std::endl;
        return;
    }
    
    // Stage returns false if no print file exists in download directory
    PurgeDirectory(testDownloadDir);
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to return false when download directory does not contain any print file archives, got true" << std::endl;
        return;
    }

    // Stage returns false if archive is not a valid .tar.gz
    Copy("/smith/test_resources/invalid.tar.gz", testDownloadDir);
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected Stage to return false when download directory contains invalid archive, got true" << std::endl;
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
        return;
    }
    
    std::string jobName = SETTINGS.GetString(JOB_NAME_SETTING);
    if (jobName != "MyPrintJob")
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to load settings from settings file (JobName == MyPrintJob), got (JobName == " <<
                jobName << ")" << std::endl;
        return;
    }
    
    // Returns true on success
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return true if no errors, got false" << std::endl;
        return;
    }
    
    // Returns false if settings file does not exist
    PurgeDirectory(testStagingDir);
    if (printData.LoadSettings())
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if settings file does not exist, got true" << std::endl;
        return;
    }
    
    // Returns false if settings cannot be loaded
    Stage("/smith/test_resources/print_with_invalid_settings.tar.gz");
    if (printData.LoadSettings())
    {
        std::cout << "%TEST_FAILED% time=0 testname=LoadSettingsTest (PrintDataUT) " <<
            "message=Expected LoadSettings to return false if settings file cannot be loaded, got true" << std::endl;
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
        return;
    }
    
    std::string sSlice1 = testStagingDir + "/slice_1.png";
    std::string sSlice2 = testStagingDir + "/slice_2.png";
    
    // Clears staging directory
    if (std::ifstream(sSlice1.c_str()) || std::ifstream(sSlice2.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData to clear staging directory, staging directory not empty" << std::endl;
        return;
    }

    // Returns true on success
    if (!success)
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData to return true on success, got false" << std::endl;
        return;
    }
    
    // Returns false if unable to copy slices
    Stage("/smith/test_resources/print.tar.gz");
    SETTINGS.Set(PRINT_DATA_DIR, "/bogus/directory");
    if (printData.MovePrintData())
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData return false when unable to copy slices, got true" << std::endl;
        return;
    }
    
    // Returns false if unable to clear staging directory
    SETTINGS.Set(STAGING_DIR, "bogus");
    if (printData.MovePrintData())
    {
        std::cout << "%TEST_FAILED% time=0 testname=MovePrintDataTest (PrintDataUT) " <<
            "message=Expected MovePrintData return false when unable to clear staging directory, got true" << std::endl;
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
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

