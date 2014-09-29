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
    
    void SendStartPrintDataLoadCommand()
    {
        std::string command("StartPrintDataLoad\n");
        int fd = open(COMMAND_PIPE, O_WRONLY);
        write(fd, command.data(), command.length());
        close(fd);
    }    
    
    void SendProcessPrintDataCommand()
    {
        std::string command("ProcessPrintData\n");
        int fd = open(COMMAND_PIPE, O_WRONLY);
        write(fd, command.data(), command.length());
        close(fd);
    }

    void EnterHomeState()
    {
        PrinterStateMachine* pPSM = printEngine.GetStateMachine();
        pPSM->process_event(EvInitialized());
        pPSM->process_event(EvAtHome());
    }

    void ProcessPrintDataSuccessfulTest() {}
    void ProcessPrintDataInInvalidStateTest() {}
    void ProcessPrintDataStageFailsTest() {}
    void ProcessPrintDataValidateFailsTest() {}
    void ProcessPrintDataLoadSettingsFailsTest() {}
    void ProcessPrintDataPurgeDirectoryFailsTest() {}
    void ProcessPrintDataMovePrintDataFailsTest() {}

    void ProcessPrintDataTest() {
        std::cout << "PE_PD_IT ProcessPrintDataTest" << std::endl;
        
        // Put printer in Home state
        // Print data can only be processed in the Home or Idle states
        EnterHomeState();
        
        // Put a print file in the download directory
        Copy("/smith/test_resources/print.tar.gz", testDownloadDir);
     
        // Send commands
        SendStartPrintDataLoadCommand();
        SendProcessPrintDataCommand();
      
        // Process event queue
        eventHandler.Begin(4);

        UISubState secondToLastUISubState = ui._UISubStates.at(ui._UISubStates.size() - 2);

        // ProcessPrintData triggers status update with Downloading UISubState
        if (secondToLastUISubState != Downloading)
        {
            std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                    << "message=Expected status update to have UISubState of Downloading when processing begins, got \"" 
                    << secondToLastUISubState << "\"" << std::endl;
            return;
        }

        // Settings are applied and print data is present
        int layerThickness = SETTINGS.GetInt(LAYER_THICKNESS);
        if (layerThickness != 10)
        {
            std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                    << "message=Expected layer thickness setting to be \"10\" when processing is successful, got \""
                    << layerThickness << "\"" << std::endl;
            return;
        }
        if (!std::ifstream((testPrintDataDir + "/slice_1.png").c_str()))
        {
            std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                    << "message=Expected print data to be present when processing is successful, print data not found in print data directory"
                    << std::endl;

            return;
        }
       
        UISubState lastUISubState = ui._UISubStates.back();
        std::string lastJobName = ui._jobNames.back();

        // ProcessPrintData triggers status update with empty UISubState and jobName corresponding to print file name
        if (lastUISubState != Downloaded)
        {
            std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                    << "message=Expected status update to have UISubState Downloaded when processing is successful, got \""
                    << lastUISubState << "\"" << std::endl;
            return;
        }
        if (lastJobName != "MyPrintJob")
        {
            std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                    << "message=Expected status update to have jobName of \"MyPrintJob\" when processing is successful, got \""
                    << lastJobName << "\"" << std::endl;
            return;
        }
    }
};

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_PD_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% ProcessPrintDataTest (PE_PD_IT)\n" << std::endl;
    {
        PE_PD_IT test;
        test.Setup();
        test.ProcessPrintDataTest();
        test.TearDown();
    }
    std::cout << "%TEST_FINISHED% time=0 ProcessPrintDataTest (PE_PD_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}