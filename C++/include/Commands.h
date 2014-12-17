/* 
 * File:   Commands.h
 * Author: Richard Greene
 *
 * Defines the commands accepted by the printer
 * 
 * Created on May 20, 2014, 12:01 PM
 */

#ifndef COMMANDS_H
#define	COMMANDS_H

#include <Logger.h>

/// The commands accepted by the printer
enum Command
{
    // undefined command
    UndefinedCommand = 0,
    
    // start a print
    Start,
    
    // cancel the current print in progress
    Cancel,
    
    // pause the current print in progress
    Pause,
    
    // resume the paused print in progress
    Resume,
    
    // perform a soft reset of the printer
    Reset,
      
    // get the status of the printer and any print in progress
    GetStatus,
    
    // show the "Downloading..." screen 
    ShowPrintDataDownloading,
    
    // show the "Download failed" screen 
    ShowPrintDownloadFailed,
    
    // verify we can accept print data and show the "Loading..." screen
    StartPrintDataLoad,
    
    // load print data and settings from print file
    ProcessPrintData,
    
    // show the data loaded screen (for use when just loading settings)
    ShowPrintDataLoaded,
    
    // get the current value of a setting
    GetSetting,
    
    // set a new value for a setting
    SetSetting,
    
    // restore the default value of a setting
    RestoreSetting,
    
    // re-load the settings from the settings file (after it's been changed))
    RefreshSettings,
    
    // apply print settings from a file
    ApplyPrintSettings,
    
    // upload the printers logs
    GetLogs,
    
    // download new printer firmware
    SetFirmware,
    
    // Show a test pattern
    Test,
    
    // Show a projector calibration image
    CalImage,
    
    // gets the firmware version string
    GetFWVersion,
    
    // gets the board serial number string
    GetBoardNum,
    
    // go to the registration screen 
    StartRegistering,
    
    // indicate that the printer is registered
    RegistrationSucceeded,
    
    // start the calibration procedure
    StartCalibration,
    
    // Quit this application
    Exit
};

// ABC defining the interface to a class that handles commands.
class ICommandTarget : public IErrorHandler
{
public:
    virtual void Handle(Command command) = 0;
};

#endif	/* COMMANDS_H */

