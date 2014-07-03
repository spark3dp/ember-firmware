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
#include <utils.h>

std::string stagingDir, downloadDir, printDataDir;

void Setup()
{
    stagingDir = CreateTempDir();
    downloadDir = CreateTempDir();
    
    SETTINGS.Set(STAGING_DIR, stagingDir);
    SETTINGS.Set(DOWNLOAD_DIR, downloadDir);
}

void TearDown()
{
    SETTINGS.Restore(STAGING_DIR);
    SETTINGS.Restore(DOWNLOAD_DIR);
    
    RemoveDir(stagingDir);
    RemoveDir(downloadDir);
    
    stagingDir = "";
    downloadDir = "";
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
    Touch(stagingDir + "/slice_2.png");
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=ValidateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain first slice image, got true" << std::endl;
        return;
    }
    
    PurgeDirectory(stagingDir);
    Touch(stagingDir + "/slice_1.png");
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
    std::string printFile = downloadDir + "/print.tar.gz";
    Copy("/smith/test_resources/print.tar.gz", printFile);
    
    // Place a stray file in the staging directory
    std::string stray_file = stagingDir + "/stray_file";
    Touch(stray_file);
    
    std::string slice_1 = stagingDir + "/slice_1.png";
    std::string slice_2 = stagingDir + "/slice_2.png";
    
    PrintData printData;
    
    bool result = printData.Stage();
    
    // Staging directory is cleared
    if (std::ifstream(stray_file.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to empty staging directory, stray file still in staging directory" << std::endl;
        return;
    }
    
    // Print file is extracted to staging directory
    if (!(std::ifstream(slice_1.c_str()) && std::ifstream(slice_2.c_str())))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to extract print job archive to staging directory, did not find extracted files" << std::endl;
        return;
    }
    
    // Print file is deleted after successful extraction
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to remove extracted print file, but print file is still in download directory" << std::endl;
        return;
    }
    
    // Job name is stored
    if (printData.GetJobName() != "print")
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to store print file name without extension as job name, got something else" << std::endl;
        return;
    }
    
    // Stage returns true if all operations are successful
    if (!result)
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
            "message=Expected stage to return true if no errors, got false" << std::endl;
        return;
    }
    
    // Stage returns false if no print file exists in download directory
    PurgeDirectory(downloadDir);
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to return false when download directory does not contain any print file archives, got true" << std::endl;
        return;
    }
    
    // Stage returns false if selected print file cannot be extracted
    Touch(downloadDir + "/invalid.tar.gz");
    if (printData.Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=StageTest (PrintDataUT) " <<
                "message=Expected stage to return false when attempting to extract invalid print file archive, got true" << std::endl;
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
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

