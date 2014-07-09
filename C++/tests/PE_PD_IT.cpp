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
#include <Filenames.h>
#include <Settings.h>
#include <PrinterStateMachine.h>

class UIProxy : public ICallback
{
    public:
        int _numCallbacks;
        std::vector<PrinterStatus> _printerStatuses;

        UIProxy() : _numCallbacks(0) {}
        
        void Reset()
        {
            _numCallbacks = 0;
            _printerStatuses.clear();
        }
        
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
                    _printerStatuses.push_back(*status);
                    std::cout << "\tprinter status index: " << _printerStatuses.size() - 1 << std::endl;
                    std::cout << "\tprinter status state: " << status->_state << std::endl;
                    std::cout << "\tprinter status change: " << status->_change << std::endl;
                    std::cout << "\tprinter status isError: " << status->_isError << std::endl;
                    std::cout << "\tprinter status errorCode: " << status->_errorCode << std::endl;
                    std::cout << "\tprinter status errorMessage: " << status->_errorMessage << std::endl;
                    std::cout << "\tprinter status jobName: " << status->_jobName << std::endl;
                    std::cout << "\tprinter status UISubState: " << status->_UISubState << std::endl;
                    break;
                default:
                    HandleImpossibleCase(eventType);
                    break;
            }
        }

};

std::string testStagingDir, testDownloadDir, testPrintDataDir;

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

void SendProcessPrintDataCommand()
{
    // Send command
    std::string command("ProcessPrintData\n");
    int fd = open(COMMAND_PIPE, O_WRONLY);
    write(fd, command.data(), command.length());
    close(fd);
}

void ProcessPrintDataTest() {
    std::cout << "PE_PD_IT ProcessPrintDataTest" << std::endl;
   
    // Put a print file in the download directory
    Copy("/smith/test_resources/print.tar.gz", testDownloadDir);
    
    // Assemble PrintEngine, EventHandler, and CommandInterpreter so UICommands
    // coming in through the command pipe are handled by the print engine
    // Subscribe UIProxy to status updates so status updates are available for assertion
    EventHandler eventHandler;
    PrintEngine printEngine(false);
    
    CommandInterpreter commandInterpreter(&printEngine);
    eventHandler.Subscribe(UICommand, &commandInterpreter);
    
    UIProxy ui;
    eventHandler.SetFileDescriptor(PrinterStatusUpdate, printEngine.GetStatusUpdateFD());
    eventHandler.Subscribe(PrinterStatusUpdate, &ui);
    
    printEngine.Begin();

    // Put printer in Home state
    // Print data can only be processed in the Home or Idle states
    PrinterStateMachine* pPSM = printEngine.GetStateMachine();
    pPSM->process_event(EvAtHome());
   
    SendProcessPrintDataCommand();
  
    // Process event queue
    eventHandler.Begin(1);

    PrinterStatus lastStatus = ui._printerStatuses.back();
    PrinterStatus secondToLastStatus = ui._printerStatuses.at(ui._printerStatuses.size() - 2);

    // ProcessPrintData triggers status update with Downloading UISubState
    if (strcmp(secondToLastStatus._UISubState, UISUBSTATE_DOWNLOADING) != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                << "message=Expected status update to have UISubState of \"" << UISUBSTATE_DOWNLOADING << "\" "
                << "when processing begins, got \"" << secondToLastStatus._UISubState << "\"" << std::endl;
        return;
    }

    // Settings are applied and print data is present
    std::string layerThickness = SETTINGS.GetString(LAYER_THICKNESS);
    if (layerThickness != "10")
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
    
    // ProcessPrintData triggers status update with empty UISubState and jobName corresponding to print file name
    if (strcmp(lastStatus._UISubState, "") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                << "message=Expected status update to have empty UISubState when processing is successful, got \""
                << lastStatus._UISubState << "\"" << std::endl;
        return;
    }
    if (strcmp(lastStatus._jobName, "print") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=ProcessPrintDataTest (PE_PD_IT) "
                << "message=Expected status update to have jobName of print when processing is successful, got \""
                << lastStatus._jobName << "\"" << std::endl;
        return;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_PD_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% ProcessPrintDataTest (PE_PD_IT)\n" << std::endl;
    Setup();
    ProcessPrintDataTest();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 ProcessPrintDataTest (PE_PD_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}