//  File:   PrintDataDirectoryUT.cpp
//  Tests PrintDataDirectory
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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

#include <sys/stat.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "support/FileUtils.hpp"
#include <PrintDataDirectory.h>

int mainReturnValue = EXIT_SUCCESS;

std::string testDataDir, testPrintDataDir;

void Setup()
{
    testDataDir = CreateTempDir();
    testPrintDataDir = CreateTempDir();
}

void TearDown()
{
    RemoveDir(testDataDir);
    RemoveDir(testPrintDataDir);
    
    testDataDir = "";
    testPrintDataDir = "";
}

void TestValidateWhenPrintDataValid()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataValid" << std::endl;

    Copy("resources/slices/slice_1.png", testDataDir);
    Copy("resources/slices/slice_2.png", testDataDir);

    PrintDataDirectory printData(testDataDir);

    if (!printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataValid (PrintDataDirectoryUT) "
                << "message=Expected validate to return true when print file contains consecutively named slice "
                << "images starting with 1, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataEmpty()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataEmpty" << std::endl;

    PrintDataDirectory printData(testDataDir);

    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataEmpty (PrintDataDirectoryUT) "
                << "message=Expected validate to return false when print file does not contain any images, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataMissingFirstSlice()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataMissingFirstSlice" << std::endl;

    Copy("resources/slices/slice_2.png", testDataDir);

    PrintDataDirectory printData(testDataDir);
    
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataMissingFirstSlice (PrintDataDirectoryUT) "
                << "message=Expected validate to return false when print file does not contain first slice image, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataHasNamingGap()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataHasNamingGap" << std::endl;

    Copy("resources/slices/slice_1.png", testDataDir);
    Copy("resources/slices/slice_2.png", testDataDir + "/slice_3.png");

    PrintDataDirectory printData(testDataDir);

    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataHasNamingGap (PrintDataDirectoryUT) "
                << "message=Expected validate to return false when print data contains slices with a naming gap, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataHasSlice0()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataHasSlice0" << std::endl;

    Copy("resources/slices/slice_1.png", testDataDir + "/slice_0.png");
    Copy("resources/slices/slice_1.png", testDataDir);

    PrintDataDirectory printData(testDataDir);
    
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataHasSlice0 (PrintDataDirectoryUT) "
                << "message=Expected validate to return false when print data contains slice 0 image, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestMoveWhenDestinationDirectoryExists()
{
    std::cout << "PrintDataDirectoryUT TestMoveWhenDestinationDirectoryExists" << std::endl;

    // To verify that the data is actually moved, the test checks for an empty parent directory
    // Create a child directory so a known parent directory exists
    std::string dataDir = testDataDir + "/dataDir";
    mkdir(dataDir.c_str(), 0755);
    
    Copy("resources/slices/slice_1.png", dataDir);
    Copy("resources/slices/slice_2.png", dataDir);
    Copy("resources/good_settings", dataDir + "/printsettings");
   
    PrintDataDirectory printData(dataDir);

    if (!printData.Move(testPrintDataDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected Move to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Look for the print data directory in its new location
    DIR* printDataDir = opendir(testPrintDataDir.c_str());
    std::string newPath;
    struct dirent* entry = readdir(printDataDir);
    
    while (entry != NULL)
    {
        if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
        {
            std::ostringstream ss;
            ss << testPrintDataDir << "/" << entry->d_name;
            newPath = ss.str();
        }
        entry = readdir(printDataDir);
    }
    
    closedir(printDataDir);

    // Old directory no longer contains print data
    if (GetEntryCount(testDataDir, DT_DIR) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected Move to remove print data from previous parent directory, directory still "
                << "present" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Specified destination contains print data
    std::string slice1 = newPath + "/slice_1.png";
    std::string slice2 = newPath + "/slice_2.png";
    std::string printSettings = newPath + "/printsettings";
    if (!std::ifstream(slice1.c_str()) || !std::ifstream(slice2.c_str()) || !std::ifstream(printSettings.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected destination directory to contain print data, all print data not present"
                << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that PrintData instance knows where its data resides after moving
    int expectedLayerCount = 2;
    int actualLayerCount = printData.GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataDirectoryUT) "
                << "message=Layer count incorrect after moving print data, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestMoveWhenDestinationDirectoryDoesNotExist()
{
    std::cout << "PrintDataDirectoryUT TestMoveWhenDestinationDirectoryDoesNotExist" << std::endl;
    
    Copy("resources/slices/slice_1.png", testDataDir);
    Copy("resources/slices/slice_2.png", testDataDir);
   
    PrintDataDirectory printData(testDataDir);

    if (printData.Move("bogus"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected Move to return false, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that PrintData instance knows where its data resides after failure to move
    int expectedLayerCount = 2;
    int actualLayerCount = printData.GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataDirectoryUT) "
                << "message=Layer count incorrect after failing to move print data, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestRemoveWhenUnderlyingDataExists()
{
    std::cout << "PrintDataDirectoryUT TestRemoveWhenUnderlyingDataExists" << std::endl;
   
    // To verify that the data is actually removed, the test checks for an empty parent directory
    // Create a child directory so a known parent directory exists
    std::string dataDir = testDataDir + "/dataDir";
    mkdir(dataDir.c_str(), 0755);
    
    Copy("resources/slices/slice_1.png", dataDir);
   
    PrintDataDirectory printData(dataDir);

    if (!printData.Remove())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataExists (PrintDataDirectoryUT) "
                << "message=Expected Remove to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that the directory no longer exists
    if (GetEntryCount(testDataDir, DT_DIR) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataExists (PrintDataDirectoryUT) "
                << "message=Expected Remove to remove print data, directory still present" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestRemoveWhenUnderlyingDataDoesNotExist()
{
    std::cout << "PrintDataDirectoryUT TestRemoveWhenUnderlyingDataDoesNotExist" << std::endl;
    
    PrintDataDirectory printData("bogus");

    if (printData.Remove())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataDirectoryUT) "
                << "message=Expected Remove to return false, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintDataDirectoryUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataValid (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataValid();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataValid (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataEmpty (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataEmpty();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataEmpty (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataMissingFirstSlice (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataMissingFirstSlice();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataMissingFirstSlice (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataHasNamingGap (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataHasNamingGap();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataHasNamingGap (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataHasSlice0 (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataHasSlice0();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataHasSlice0 (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestMoveWhenDestinationDirectoryExists();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT)" << std::endl;
 
    std::cout << "%TEST_STARTED% TestMoveWhenDestinationDirectoryDoesNotExist (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestMoveWhenDestinationDirectoryDoesNotExist();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestMoveWhenDestinationDirectoryDoesNotExist (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestRemoveWhenUnderlyingDataExists (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestRemoveWhenUnderlyingDataExists();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestRemoveWhenUnderlyingDataExists (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataDirectoryUT)" << std::endl;
    Setup();
    TestRemoveWhenUnderlyingDataDoesNotExist();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataDirectoryUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

