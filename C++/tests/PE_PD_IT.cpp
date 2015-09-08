//  File:   PE_PD_IT.cpp
//  PrintEngine/PrintData integration test
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
#include <fcntl.h>

#include "Timer.h"
#include "PrinterStatusQueue.h"
#include "Settings.h"
#include "PrinterStatus.h"
#include "EventHandler.h"
#include "Motor.h"
#include "PrintEngine.h"
#include "CommandInterpreter.h"
#include "PrinterStateMachine.h"
#include "Filenames.h"
#include "CommandPipe.h"

int mainReturnValue = EXIT_SUCCESS;

class UIProxy : public ICallback
{
    public:
        int _numCallbacks;
        std::vector<UISubState> _UISubStates;
        std::vector<std::string> _jobNames;

        UIProxy() : _numCallbacks(0) {}
        
    private:
        void Callback(EventType eventType, const EventData& data)
        {
            PrinterStatus status;
            switch(eventType)
            {
                case PrinterStatusUpdate:
                    std::cout << "UI proxy received printer status update" << std::endl;
                    _numCallbacks++;
                    status = data.Get<PrinterStatus>();
                    _UISubStates.push_back(status._UISubState);
                    _jobNames.push_back(SETTINGS.GetString(JOB_NAME_SETTING));
                    std::cout << "\tprinter status index: " << _UISubStates.size() - 1 << std::endl;
                    std::cout << "\tprinter status state: " << STATE_NAME(status._state) << std::endl;
                    std::cout << "\tprinter status UISubState: " << status._UISubState << std::endl;
                    std::cout << "\tprinter status change: " << status._change << std::endl;
                    std::cout << "\tprinter status isError: " << status._isError << std::endl;
                    std::cout << "\tprinter status errorCode: " << status._errorCode << std::endl;
                    std::cout << "\tprinter status errno: " << status._errno << std::endl;
                    break;
                default:
                    std::cout << "UIProxy: impossible case" << std::endl;
                    break;
            }
        }

};

class PE_PD_IT
{
public:
    std::string testStagingDir, testDownloadDir, testPrintDataDir;
    EventHandler eventHandler;
    UIProxy ui;
    Motor motor;
    PrinterStatusQueue printerStatusQueue;
    CommandPipe commandPipe;
    Timer timer1;
    Timer timer2;
    Timer timer3;
    Timer timer4;
    PrintEngine printEngine;
    CommandInterpreter commandInterpreter;
   
    PE_PD_IT() :
    eventHandler(),
    motor(0xFF), // 0xFF results in "null" I2C device that does not actually write to the bus
    printEngine(false, motor, printerStatusQueue, timer1, timer2, timer3, timer4),
    commandInterpreter(&printEngine),
    ui()
    {
        // Assemble PrintEngine, EventHandler, and CommandInterpreter so UICommands
        // coming in through the command pipe are handled by the print engine
        // Subscribe UIProxy to status updates so status updates are available for assertion

        eventHandler.AddEvent(UICommand, &commandPipe);
        eventHandler.AddEvent(PrinterStatusUpdate, &printerStatusQueue);
        
        eventHandler.Subscribe(UICommand, &commandInterpreter);
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
    
    void TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent" << std::endl;
       
        // Ensure that no temp settings file exists
        remove(TEMP_SETTINGS_FILE);
        
        // Put a print file in the download directory
        Copy("resources/print.tar.gz", testDownloadDir);
        
        // Set a print setting not contained in the print settings file
        SETTINGS.Set(MODEL_EXPOSURE, 50.0);

        ProcessPrintData();

        UISubState secondToLastUISubState = ui._UISubStates.at(ui._UISubStates.size() - 2);
        
        // ProcessPrintData triggers status update with Downloading UISubState
        if (secondToLastUISubState != LoadingPrintData)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have UISubState of Downloading when processing begins, got \"" 
                    << secondToLastUISubState << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        // Settings are applied and print data is present
        int layerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
        if (layerThickness != 10)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected layer thickness setting to be \"10\" when processing is successful, got \""
                    << layerThickness << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        if (!printEngine.HasAtLeastOneLayer())
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected print data to be present when processing is successful, print data not present"
                    << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
       
        UISubState lastUISubState = ui._UISubStates.back();
        std::string lastJobName = ui._jobNames.back();

        // ProcessPrintData triggers status update with empty UISubState and jobName corresponding to print file name
        if (lastUISubState != LoadedPrintData)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have UISubState Downloaded when processing is successful, got \""
                    << lastUISubState << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        if (lastJobName != "MyPrintJob")
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected status update to have jobName of \"MyPrintJob\" when processing is successful, got \""
                    << lastJobName << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        // Verify that processing print data restores print settings
        float expectedModelExposure = 2.5;
        float actualModelExposure = SETTINGS.GetDouble(MODEL_EXPOSURE);
        if (expectedModelExposure != actualModelExposure)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected restoration of print settings when processing print data, expected model exposure to equal "
                    << expectedModelExposure << ", got " << actualModelExposure << std::endl;
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

    void TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent" << std::endl;

        // Ensure that no temp settings file exists
        remove(TEMP_SETTINGS_FILE);

        // Put a print file not containing a settings file in the download directory
        Copy("resources/print_with_no_settings.tar.gz", testDownloadDir);
        
        ProcessPrintData();

        // Error condition handled by entering appropriate UISubState
        UISubState expectedUISubState = PrintDataLoadFailed;
        UISubState actualUISubState = ui._UISubStates.back();
        if (expectedUISubState != actualUISubState)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected UISubState to equal " << expectedUISubState << " when settings file not present, got "
                    << actualUISubState << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }

    void TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent" << std::endl;

        // Put a print file in the download directory
        Copy("resources/print.zip", testDownloadDir);
        
        ProcessPrintData();

        // Settings are applied and print data is present
        int layerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
        if (layerThickness != 10)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected layer thickness setting to be \"10\" when processing is successful, got \""
                    << layerThickness << "\"" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        if (!printEngine.HasAtLeastOneLayer())
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected print data to be present when processing is successful, print data not present"
                    << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }

    void TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent()
    {
        std::cout << "PE_PD_IT TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent" << std::endl;

        // Ensure that no temp settings file exists
        remove(TEMP_SETTINGS_FILE);

        // Put a print file not containing a settings file in the download directory
        Copy("resources/print_with_no_settings.zip", testDownloadDir);
        
        ProcessPrintData();

        // Error condition handled by entering appropriate UISubState
        UISubState expectedUISubState = PrintDataLoadFailed;
        UISubState actualUISubState = ui._UISubStates.back();
        if (expectedUISubState != actualUISubState)
        {
            std::cout << "%TEST_FAILED% time=0 testname=TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent (PE_PD_IT) "
                    << "message=Expected UISubState to equal " << expectedUISubState << " when settings file not present, got "
                    << actualUISubState << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }
};

int main(int argc, char** argv)
{
    std::cout << "%SUITE_STARTING% PE_PD_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenPrintFileIsTarGzWhenTempSettingsFileNotPresent (PE_PD_IT)" << std::endl;

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
 
    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenPrintFileIsTarGzWhenSettingsFileNotPresent (PE_PD_IT)" << std::endl;
    
    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenPrintFileIsZipWhenTempSettingsFileNotPresent (PE_PD_IT)" << std::endl;
 
    std::cout << "%TEST_STARTED% TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 TestProcessPrintDataWhenPrintFileIsZipWhenSettingsFileNotPresent (PE_PD_IT)" << std::endl;
     
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}
