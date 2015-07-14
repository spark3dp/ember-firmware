/* 
 * 
#include "PrinterStateMachine.h"File:   PE_PD_IT.cpp
 * Author: Jason Lefley
 * PrintEngine PrintData integration test
 *
 * Created on Jul 7, 2014, 4:49:06 PM
 */

#include <stdlib.h>
#include <iostream>
#include <fcntl.h>

#include <PrintEngine.h>
#include <EventHandler.h>
#include <CommandInterpreter.h>
#include <Shared.h>
#include <Settings.h>
#include <PrinterStateMachine.h>

int mainReturnValue = EXIT_SUCCESS;

class UIProxy : public ICallback
{
    public:
        int _numCallbacks;
        std::vector<UISubState> _UISubStates;
        std::vector<std::string> _jobNames;

        UIProxy() : _numCallbacks(0) {}
        
    private:
        void Callback(EventType eventType, void* data)
        {
            PrinterStatus* status;
            switch(eventType)
            {
                case PrinterStatusUpdate:
                    std::cout << "UI proxy received printer status update" << std::endl;
                    _numCallbacks++;
                    status = (PrinterStatus*)data;
                    _UISubStates.push_back(status->_UISubState);
                    _jobNames.push_back(SETTINGS.GetString(JOB_NAME_SETTING));
                    std::cout << "\tprinter status index: " << _UISubStates.size() - 1 << std::endl;
                    std::cout << "\tprinter status state: " << STATE_NAME(status->_state) << std::endl;
                    std::cout << "\tprinter status UISubState: " << status->_UISubState << std::endl;
                    std::cout << "\tprinter status change: " << status->_change << std::endl;
                    std::cout << "\tprinter status isError: " << status->_isError << std::endl;
                    std::cout << "\tprinter status errorCode: " << status->_errorCode << std::endl;
                    std::cout << "\tprinter status errno: " << status->_errno << std::endl;
                    break;
                default:
                    HandleImpossibleCase(eventType);
                    break;
            }
        }

};

class PE_PD_IT
{
public:
    std::string testStagingDir, testDownloadDir, testPrintDataDir;
    EventHandler eventHandler;
    PrintEngine printEngine;
    CommandInterpreter commandInterpreter;
    UIProxy ui;
   
    PE_PD_IT() :
    eventHandler(),
    printEngine(false),
    commandInterpreter(&printEngine),
    ui()
    {
        // Assemble PrintEngine, EventHandler, and CommandInterpreter so UICommands
        // coming in through the command pipe are handled by the print engine
        // Subscribe UIProxy to status updates so status updates are available for assertion
        eventHandler.Subscribe(UICommand, &commandInterpreter);
        eventHandler.SetFileDescriptor(PrinterStatusUpdate, printEngine.GetStatusUpdateFD());
        eventHandler.Subscribe(PrinterStatusUpdate, &ui);
        printEngine.Begin();
    }
    
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

    void ProcessPrintData()
    {
        // Put printer in Home state
        // Print data can only be processed in the Home state
        PrinterStateMachine* pPSM = printEngine.GetStateMachine();
        pPSM->process_event(EvInitialized());
        pPSM->process_event(EvMotionCompleted());
     
        // Send commands
        std::string startPrintDataLoadCommand("StartPrintDataLoad\n");
        std::string processPrintDataCommand("ProcessPrintData\n");

        int fd = open(COMMAND_PIPE, O_WRONLY);
        write(fd, startPrintDataLoadCommand.data(), startPrintDataLoadCommand.length());
        write(fd, processPrintDataCommand.data(), processPrintDataCommand.length());
        close(fd);
        
        // Process event queue
        eventHandler.Begin(4);
    }
    
    void TestProcessPrintDataWhenTempSettingsFileNotPresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenTempSettingsFileNotPresent" << std::endl;
       
        // Ensure that no temp settings file exists
        remove(TEMP_SETTINGS_FILE);
        
        // Put a print file in the download directory
        Copy("resources/print.tar.gz", testDownloadDir);
        
        ProcessPrintData();

        UISubState secondToLastUISubState = ui._UISubStates.at(ui._UISubStates.size() - 2);

        // ProcessPrintData triggers status update with Downloading UISubState
        if (secondToLastUISubState != LoadingPrintData)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have UISubState of Downloading when processing begins, got \"" 
                    << secondToLastUISubState << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        // Settings are applied and print data is present
        int layerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
        if (layerThickness != 10)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected layer thickness setting to be \"10\" when processing is successful, got \""
                    << layerThickness << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        if (!std::ifstream((testPrintDataDir + "/slice_1.png").c_str()))
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected print data to be present when processing is successful, print data not found in print data directory"
                    << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
       
        UISubState lastUISubState = ui._UISubStates.back();
        std::string lastJobName = ui._jobNames.back();

        // ProcessPrintData triggers status update with empty UISubState and jobName corresponding to print file name
        if (lastUISubState != LoadedPrintData)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have UISubState Downloaded when processing is successful, got \""
                    << lastUISubState << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        if (lastJobName != "MyPrintJob")
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have jobName of \"MyPrintJob\" when processing is successful, got \""
                    << lastJobName << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }
    
    void TestProcessPrintDataWhenTempSettingsFilePresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenTempSettingsFilePresent" << std::endl;

        // Put a temp settings file in place
        remove(TEMP_SETTINGS_FILE);
        Copy("resources/good_settings", TEMP_SETTINGS_FILE);
        
        // Put a print file in the download directory
        Copy("resources/print.tar.gz", testDownloadDir);
       
        ProcessPrintData();
        
        // PrintEngine applies settings from temp settings file rather than settings included with print file
        std::string actualJobName = SETTINGS.GetString(JOB_NAME_SETTING);
        std::string expectedJobName = "NewJobName";
        if (actualJobName != expectedJobName)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFilePresent (PE_PD_IT) "
                    << "message=Expected job name to equal \"" << expectedJobName << "\", got \"" << actualJobName << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        // PrintEngine removes temp settings file after processing print data
        std::ifstream tempSettingsFile(TEMP_SETTINGS_FILE);
        if (tempSettingsFile.good())
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFilePresent (PE_PD_IT) "
                    << "message=temp settings file not removed after loading print data" << std::endl;
            remove(TEMP_SETTINGS_FILE);
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }

    void TestProcessPrintDataWhenPrintFileInvalid()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenPrintFileInvalid" << std::endl;
       
        // Put a temp settings file in place
        remove(TEMP_SETTINGS_FILE);
        Copy("resources/good_settings", TEMP_SETTINGS_FILE);

        // Put a print file containing invalid settings in the download directory
        Copy("resources/print_with_invalid_settings.tar.gz", testDownloadDir);
       
        ProcessPrintData();
        
        // PrintEngine removes temp settings file when print file invalid
        std::ifstream tempSettingsFile(TEMP_SETTINGS_FILE);
        if (tempSettingsFile.good())
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileInvalid (PE_PD_IT) "
                    << "message=temp settings file not removed after loading print data" << std::endl;
            remove(TEMP_SETTINGS_FILE);
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }

    void TestProcessPrintDataWhenTempSettingsFileInvalid()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenTempSettingsFileInvalid" << std::endl;

        // Put a temp settings file in place
        remove(TEMP_SETTINGS_FILE);
        Copy("resources/bad_settings", TEMP_SETTINGS_FILE);
        
        // Put a print file in the download directory
        Copy("resources/print.tar.gz", testDownloadDir);
       
        ProcessPrintData();
        
        // PrintEngine removes temp settings file when temp settings file invalid
        std::ifstream tempSettingsFile(TEMP_SETTINGS_FILE);
        if (tempSettingsFile.good())
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenTempSettingsFileInvalid (PE_PD_IT) "
                    << "message=temp settings file not removed after loading print data" << std::endl;
            remove(TEMP_SETTINGS_FILE);
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }
};

int main(int argc, char** argv)
{
    std::cout << "%SUITE_STARTING% PE_PD_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenTempSettingsFileNotPresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenTempSettingsFileNotPresent (PE_PD_IT)" << std::endl;

    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenTempSettingsFilePresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenTempSettingsFilePresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenTempSettingsFilePresent (PE_PD_IT)" << std::endl;

    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenPrintFileInvalid (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenPrintFileInvalid();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenPrintFileInvalid (PE_PD_IT)" << std::endl;

    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenTempSettingsFileInvalid (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenTempSettingsFileInvalid();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenTempSettingsFileInvalid (PE_PD_IT)" << std::endl;
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}
