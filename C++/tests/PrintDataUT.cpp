//  File:   PrintDataUT.cpp
//  Tests PrintData
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

#include <boost/scoped_ptr.hpp>

#include "support/FileUtils.hpp"
#include <PrintData.h>
#include "PrintFileStorage.h"

int mainReturnValue = EXIT_SUCCESS;

std::string testStagingDir, testDownloadDir, testPrintDataDir;

void Setup()
{
    testStagingDir = CreateTempDir();
    testDownloadDir = CreateTempDir();
    testPrintDataDir = CreateTempDir();
}

void TearDown()
{
    RemoveDir(testStagingDir);
    RemoveDir(testDownloadDir);
    RemoveDir(testPrintDataDir);
    
    testStagingDir = "";
    testDownloadDir = "";
    testPrintDataDir = "";
}

void TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile" << std::endl;

    // Put a print file archive in the download directory
    Copy("resources/print.tar.gz", testDownloadDir);

    // Put a stray file in the staging directory
    std::string strayFile = testStagingDir + "/stray_file";
    Touch(strayFile);

    PrintFileStorage storage(testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(storage, testStagingDir, "new_name"));

    // successfully instantiates a PrintData object
    if (!pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT) "
                << "message=got NULL pointer when attempting to create instance from tar.gz file"
                << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
   
    // successfully determines file name
    std::string expectedFileName = "print.tar.gz";
    std::string actualFileName = storage.GetFileName();
    if (expectedFileName != actualFileName)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT) "
                << "message=PrintData instance created from tar.gz file has incorrect name; expected \"" <<
                expectedFileName << "\", got \"" << actualFileName << "\"" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // clears the specified data directory
    if (std::ifstream(strayFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT) "
                << "message=specified data directory not cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // extracts the archive (layer count is determined by looking for slice files after extracting)
    int expectedLayerCount = 2;
    int actualLayerCount = pPrintData->GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT) "
                << "message=Layer count incorrect after creating PrintData instance, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // removes the archive
    std::string printFile = testDownloadDir + "/print.tar.gz";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT) "
                << "message=original print file not removed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromNewDataWhenTarGzFileCorrupt()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenTarGzFileCorrupt" << std::endl;
    
    Copy("resources/corrupt.tar.gz", testDownloadDir);
   
    PrintFileStorage storage(testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(storage, testStagingDir, "new_name"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // removes the archive
    std::string printFile = testDownloadDir + "/corrupt.tar.gz";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=original print file not removed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // leaves specified data directory empty
    if (GetEntryCount(testStagingDir, DT_DIR) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=specified data directory not empty after failing to create PrintData instance" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile" << std::endl;

    // Put a print file archive in the download directory
    Copy("resources/print.zip", testDownloadDir);

    // Put a stray file in the staging directory
    std::string strayFile = testStagingDir + "/stray_file";
    Touch(strayFile);

    PrintFileStorage storage(testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(storage, testStagingDir, "new_name"));

    // successfully instantiates a PrintData object
    if (!pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT) "
                << "message=got NULL pointer when attempting to create instance from zip file"
                << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
   
    // successfully determines file name
    std::string expectedFileName = "print.zip";
    std::string actualFileName = storage.GetFileName();
    if (expectedFileName != actualFileName)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT) "
                << "message=PrintData instance created from zip file has incorrect name; expected \"" <<
                expectedFileName << "\", got \"" << actualFileName << "\"" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // clears the specified data directory
    if (std::ifstream(strayFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT) "
                << "message=specified data directory not cleared" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // able to access contained data (layer count is determined by looking for slice files in zip file)
    int expectedLayerCount = 2;
    int actualLayerCount = pPrintData->GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT) "
                << "message=Layer count incorrect after creating PrintData instance, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // removes the archive
    std::string printFile = testDownloadDir + "/print.zip";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT) "
                << "message=original print file not removed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromNewDataWhenZipFileCorrupt()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenZipFileCorrupt" << std::endl;
    
    Copy("resources/corrupt.zip", testDownloadDir);
    
    PrintFileStorage storage(testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(storage, testStagingDir, "new_name"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenZipFileCorrupt (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // removes the archive
    std::string printFile = testDownloadDir + "/corrupt.zip";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenZipFileCorrupt (PrintDataUT) "
                << "message=original print file not removed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // leaves specified data directory empty
    if (GetEntryCount(testStagingDir, DT_REG) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenZipFileCorrupt (PrintDataUT) "
                << "message=specified data directory not empty after failing to create PrintData instance" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromNewDataWhenDownloadDirectoryEmpty()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenDownloadDirectoryEmpty" << std::endl;

    PrintFileStorage storage(testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(storage, testStagingDir, "new_name"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenFileDoesNotExist()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenFileDoesNotExist" << std::endl;

    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData(testDownloadDir + "/some_print"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenFileDoesNotExist (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenSpecifiedFileNotADirectory()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenSpecifiedFileNotADirectory" << std::endl;

    Copy("resources/print.tar.gz", testDownloadDir);
    
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData(testDownloadDir + "/print.tar.gz"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenSpecifiedFileAZipFile()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenSpecifiedFileAZipFile" << std::endl;

    Copy("resources/print.zip", testDownloadDir);
    
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData(testDownloadDir + "/print.zip"));

    if (!pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenSpecifiedFileAZipFile (PrintDataUT) "
                << "message=got NULL pointer when attempting to create instance from zip file"
                << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    int expectedLayerCount = 2;
    int actualLayerCount = pPrintData->GetLayerCount();
    if (expectedLayerCount != actualLayerCount)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenSpecifiedFileAZipFile (PrintDataUT) "
                << "message=Layer count incorrect after creating PrintData instance, expected "
                << expectedLayerCount << ", got " << actualLayerCount << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile" << std::endl;
    
    Copy("resources/corrupt.zip", testDownloadDir);
    
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData(testDownloadDir + "/corrupt.zip"));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintDataUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenTarGzFileCorrupt();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenDownloadDirectoryContainsZipFile (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenZipFileCorrupt (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenZipFileCorrupt();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenZipFileCorrupt (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenDownloadDirectoryEmpty();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenFileDoesNotExist (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenFileDoesNotExist();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenFileDoesNotExist (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenSpecifiedFileNotADirectory();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenSpecifiedFileAZipFile (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenSpecifiedFileAZipFile();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenSpecifiedFileAZipFile (PrintDataUT)" << std::endl;
    
    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenSpecifiedFileACorruptZipFile (PrintDataUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}
