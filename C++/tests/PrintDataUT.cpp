/* 
 * File:   PrintDataUT.cpp
 * Author: Jason Lefley
 *
 * Created on Jun 27, 2014, 1:55:50 PM
 */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <PrintData.h>
#include <Filenames.h>

/*
 * Simple C++ Test Suite
 */
void Touch(const char* parent, const char* name)
{
    // Create an empty file with specified name in specified parent directory
    std::string fullPath = std::string(parent) + "/" + std::string(name);
    std::ofstream slice(fullPath.c_str());
    slice.close();
}

void Purge(const char* directory)
{
    struct dirent* nextFile;
    DIR* folder;
    char filePath[PATH_MAX];
    
    folder = opendir(directory);
    
    while (nextFile = readdir(folder))
    {
        sprintf(filePath, "%s/%s", directory, nextFile->d_name);
        remove(filePath);
    }
}

void validateTest()
{
    std::cout << "PrintDataUT validate test" << std::endl;
   
    // Staging folder containing no images fails validation
    //Purge(STAGING_FOLDER);
    if (PrintData::Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=validateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain any images, got true" << std::endl;
        return;
    }
    
    // Staging folder not containing first slice fails validation
    //Purge(STAGING_FOLDER);
    //Touch(STAGING_FOLDER, "slice_0002.png");
    if (PrintData::Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=validateTest (PrintDataUT) " <<
                "message=Expected validation to return false when staging folder does not contain first slice image, got true" << std::endl;
        return;
    }
    
    //Purge(STAGING_FOLDER);
    //Touch(STAGING_FOLDER, "slice_0001.png");
    if (!PrintData::Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=validateTest (PrintDataUT) " <<
                "message=Expected validation to return true when staging folder contains first slice image, got false" << std::endl;
        return;
    }
}

void stageTest() {
    std::cout << "PrintDataUT stage test" << std::endl;
    
    // Pre-test make step copies valid archive to DOWNLOAD_FOLDER
    
    //Purge(STAGING_FOLDER);
    PrintData::Stage();
    
    std::string slice_1 = std::string("/slice_0001.png");
    std::string slice_2 = std::string("/slice_0002.png");
    
    if (!(std::ifstream(slice_1.c_str()) && std::ifstream(slice_2.c_str())))
    {
        std::cout << "%TEST_FAILED% time=0 testname=stageTest (PrintDataUT) " <<
                "message=Expected stage to extract print job archive to staging directory, did not find extracted files" << std::endl;
        return;
    }
    
    //Purge(STAGING_FOLDER);
    //Purge(DOWNLOAD_FOLDER);
    if (PrintData::Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=stageTest (PrintDataUT) " <<
                "message=Expected stage to return false when download folder does not contain any print file archives, got true" << std::endl;
        return;
    }
    
    //Touch(DOWNLOAD_FOLDER, "invalid.tar.gz");
    if (PrintData::Stage())
    {
        std::cout << "%TEST_FAILED% time=0 testname=stageTest (PrintDataUT) " <<
                "message=Expected stage to return false when attempting to extract invalid print file archive, got true" << std::endl;
        return;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintDataUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% validateTest (PrintDataUT)" << std::endl;
    validateTest();
    std::cout << "%TEST_FINISHED% time=0 validateTest (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% stageTest (PrintDataUT)" << std::endl;
    stageTest();
    std::cout << "%TEST_FINISHED% time=0 stageTest (PrintDataUT)" << std::endl;
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

