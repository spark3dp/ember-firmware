/* 
 * File:   PrintDataDirectoryUT.cpp
 * Author: Jason Lefley
 *
 * Created on Jul 16, 2015, 4:23:36 PM
 */

#include <boost/scoped_ptr.hpp>
#include <sstream>

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

void TestValidateWhenPrintDataValid()
{
    std::cout << "PrintDataDirectoryUT TestValidateWhenPrintDataValid" << std::endl;

    Copy("resources/print.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (!pPrintData->Validate())
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

    Copy("resources/print_no_slices.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData->Validate())
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

    Copy("resources/print_missing_first_slice.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData->Validate())
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

    Copy("resources/print_naming_gap.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));


    if (pPrintData->Validate())
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

    Copy("resources/print_has_slice_0.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData->Validate())
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
    
    Copy("resources/print.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (!pPrintData->Move(testPrintDataDir))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected Move to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

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
    if (GetSubdirectoryCount(testStagingDir) != 0)
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
    int actualLayerCount = pPrintData->GetLayerCount();
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
    
    Copy("resources/print.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (pPrintData->Move("bogus"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestMoveWhenDestinationDirectoryExists (PrintDataDirectoryUT) "
                << "message=Expected Move to return false, got true" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that PrintData instance knows where its data resides after failure to move
    int expectedLayerCount = 2;
    int actualLayerCount = pPrintData->GetLayerCount();
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
    
    Copy("resources/print.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));

    if (!pPrintData->Remove())
    {
        std::cout << "%TEST_FAILED% time=0 testname=TestRemoveWhenUnderlyingDataExists (PrintDataDirectoryUT) "
                << "message=Expected Remove to return true, got false" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }

    // Verify that the directory no longer exists
    if (GetSubdirectoryCount(testStagingDir) != 0)
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
    
    Copy("resources/print.tar.gz", testDownloadDir);
    boost::scoped_ptr<PrintData> pPrintData(PrintData::CreateFromNewData(testDownloadDir, testStagingDir));
    
    pPrintData->Remove();

    if (pPrintData->Remove())
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

