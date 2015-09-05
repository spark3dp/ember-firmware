//  File:   PrintDataZipUT.cpp
//  Tests PrintDataZip
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

#include <PrintDataZip.h>

int mainReturnValue = EXIT_SUCCESS;

std::string testDir;

void Setup()
{
    testDir = CreateTempDir();
}

void TearDown()
{
    RemoveDir(testDir);
    
    testDir = "";
}

void TestValidateWhenPrintDataValid()
{
    std::cout << "PrintDataZipUT TestValidateWhenPrintDataValid" << std::endl;

    Copy("resources/print.zip", testDir);

    PrintDataZip printData(testDir + "/print.zip");
    
    if (!printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataValid (PrintDataZipUT) "
                << "message=Expected validate to return true when print file contains consecutively named slice "
                << "images starting with 1, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataEmpty()
{
    std::cout << "PrintDataZipUT TestValidateWhenPrintDataEmpty" << std::endl;

    Copy("resources/print_no_slices.zip", testDir);

    PrintDataZip printData(testDir + "/print_no_slices.zip");

    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataEmpty (PrintDataZipUT) "
                << "message=Expected validate to return false when print file does not contain any images, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataMissingFirstSlice()
{
    std::cout << "PrintDataZipUT TestValidateWhenPrintDataMissingFirstSlice" << std::endl;

    Copy("resources/print_missing_first_slice.zip", testDir);

    PrintDataZip printData(testDir + "/print_missing_first_slice.zip");
    
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataMissingFirstSlice (PrintDataZipUT) "
                << "message=Expected validate to return false when print file does not contain first slice image, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataHasNamingGap()
{
    std::cout << "PrintDataZipUT TestValidateWhenPrintDataHasNamingGap" << std::endl;

    Copy("resources/print_naming_gap.zip", testDir);

    PrintDataZip printData(testDir + "/print_naming_gap.zip");

    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataHasNamingGap (PrintDataZipUT) "
                << "message=Expected validate to return false when print data contains slices with a naming gap, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestValidateWhenPrintDataHasSlice0()
{
    std::cout << "PrintDataZipUT TestValidateWhenPrintDataHasSlice0" << std::endl;

    Copy("resources/print_has_slice_0.zip", testDir);

    PrintDataZip printData(testDir + "/print_has_slice_0.zip");
    
    if (printData.Validate())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestValidateWhenPrintDataHasSlice0 (PrintDataZipUT) "
                << "message=Expected validate to return false when print data contains slice 0 image, "
                << "got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestMoveWhenDestinationDirectoryExists()
{
    std::cout << "PrintDataZipUT TestMoveWhenDestinationDirectoryExists" << std::endl;
   
    // Make a destination directory
    std::string destinationDir = testDir + "/destination";
    mkdir(destinationDir.c_str(), 0755);
    
    Copy("resources/print.zip", testDir);

    PrintDataZip printData(testDir + "/print.zip");
    
    if (!printData.Move(destinationDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Expected Move to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Old directory no longer contains print data
    std::string printFile = testDir + "/print.zip";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Expected Move to remove print data from previous parent directory, directory still "
                << "present" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Specified destination contains print data
    std::string printDataFile = destinationDir + "/print.zip";
    if (!std::ifstream(printDataFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Expected destination directory to contain print data, all print data not present"
                << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that PrintData instance knows where its data resides after moving
    // Attempt to move again
    std::string destinationDir2 = testDir + "/destination2";
    mkdir(destinationDir2.c_str(), 0755);

    if (!printData.Move(destinationDir2))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Unable to move print data twice" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Access to zip file contents ok
    int expectedLayerCount = 2;
    int actualLayerCount = printData.GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Layer count incorrect after moving print data, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestMoveWhenDestinationDirectoryDoesNotExist()
{
    std::cout << "PrintDataZipUT TestMoveWhenDestinationDirectoryDoesNotExist" << std::endl;
    
    Copy("resources/print.zip", testDir);

    PrintDataZip printData(testDir + "/print.zip");

    if (printData.Move("bogus"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataZipUT) "
                << "message=Expected Move to return false, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that PrintData instance knows where its data resides after failure to move
    int expectedLayerCount = 2;
    int actualLayerCount = printData.GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataZipUT) "
                << "message=Layer count incorrect after failing to move print data, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestRemoveWhenUnderlyingDataExists()
{
    std::cout << "PrintDataZipUT TestRemoveWhenUnderlyingDataExists" << std::endl;
    
    Copy("resources/print.zip", testDir);

    PrintDataZip printData(testDir + "/print.zip");

    if (!printData.Remove())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataExists (PrintDataZipUT) "
                << "message=Expected Remove to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    std::string printFile = testDir + "/print.zip";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataExists (PrintDataZipUT) "
                << "message=Expected Remove to remove print data, file still present" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestRemoveWhenUnderlyingDataDoesNotExist()
{
    std::cout << "PrintDataZipUT TestRemoveWhenUnderlyingDataDoesNotExist" << std::endl;
 
    Copy("resources/print.zip", testDir);

    PrintDataZip printData(testDir + "/print.zip");
    
    printData.Remove();

    if (printData.Remove())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataZipUT) "
                << "message=Expected Remove to return false, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintDataZipUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataValid (PrintDataZipUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataValid();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataValid (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataEmpty (PrintDataZipUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataEmpty();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataEmpty (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataMissingFirstSlice (PrintDataZipUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataMissingFirstSlice();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataMissingFirstSlice (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataHasNamingGap (PrintDataZipUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataHasNamingGap();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataHasNamingGap (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestValidateWhenPrintDataHasSlice0 (PrintDataZipUT)" << std::endl;
    Setup();
    TestValidateWhenPrintDataHasSlice0();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestValidateWhenPrintDataHasSlice0 (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestMoveWhenDestinationDirectoryExists (PrintDataZipUT)" << std::endl;
    Setup();
    TestMoveWhenDestinationDirectoryExists();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestMoveWhenDestinationDirectoryExists (PrintDataZipUT)" << std::endl;
 
    std::cout << "%TEST_STARTED% TestMoveWhenDestinationDirectoryDoesNotExist (PrintDataZipUT)" << std::endl;
    Setup();
    TestMoveWhenDestinationDirectoryDoesNotExist();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestMoveWhenDestinationDirectoryDoesNotExist (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestRemoveWhenUnderlyingDataExists (PrintDataZipUT)" << std::endl;
    Setup();
    TestRemoveWhenUnderlyingDataExists();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestRemoveWhenUnderlyingDataExists (PrintDataZipUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataZipUT)" << std::endl;
    Setup();
    TestRemoveWhenUnderlyingDataDoesNotExist();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestRemoveWhenUnderlyingDataDoesNotExist (PrintDataZipUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

