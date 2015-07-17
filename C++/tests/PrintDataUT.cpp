/* 
 * File:   PrintDataUT.cpp
 * Author: Jason Lefley
 *
 * Created on Jun 27, 2014, 1:55:50 PM
 */

#include <boost/scoped_ptr.hpp>

#include <Settings.h>
#include <PrintData.h>

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

void TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenDownloadDirectoryContainsTarGzFile" << std::endl;

    // Put a print file archive in the download directory
    Copy("resources/print.tar.gz", testDownloadDir);

    // Put a stray file in the staging directory
    std::string strayFile = testStagingDir + "/stray_file";
    Touch(strayFile);

    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

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
    std::string actualFileName = pPrintData->GetFileName();
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
    
    Copy("resources/invalid.tar.gz", testDownloadDir);
    
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
    
    // removes the archive
    std::string printFile = testDownloadDir + "/invalid.tar.gz";
    if (std::ifstream(printFile.c_str()))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=original print file not removed" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // leaves specified data directory empty
    if (GetSubdirectoryCount(testStagingDir) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenTarGzFileCorrupt (PrintDataUT) "
                << "message=specified data directory not empty after failing to create PrintData instance" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromNewDataWhenDownloadDirectoryEmpty()
{
    std::cout << "PrintDataUT TestCreateFromNewDataWhenDownloadDirectoryEmpty" << std::endl;

    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenFileNameEmpty()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenFileNameEmpty" << std::endl;

    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData("", ""));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenFileNameEmpty (PrintDataUT) "
                << "message=did not get NULL pointer" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void TestCreateFromExistingDataWhenSpecifiedFileNotADirectory()
{
    std::cout << "PrintDataUT TestCreateFromExistingDataWhenSpecifiedFileNotADirectory" << std::endl;

    Copy("resources/print.tar.gz", testDownloadDir);
    
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromExistingData("print.tar.gz", testDownloadDir));

    if (pPrintData)
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT) "
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

    std::cout << "%TEST_STARTED% TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromNewDataWhenDownloadDirectoryEmpty();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromNewDataWhenDownloadDirectoryEmpty (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenFileNameEmpty (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenFileNameEmpty();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenFileNameEmpty (PrintDataUT)" << std::endl;

    std::cout << "%TEST_STARTED% TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT)" << std::endl;
    Setup();
    TestCreateFromExistingDataWhenSpecifiedFileNotADirectory();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 TestCreateFromExistingDataWhenSpecifiedFileNotADirectory (PrintDataUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}
