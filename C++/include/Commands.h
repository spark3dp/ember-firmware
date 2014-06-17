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

#include <limits.h>

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
    
    // start, pause, or resume a print, depending on its current state
    StartPauseOrResume,
    
    // get the status of the printer and any print in progress
    GetStatus,
    
    // download the data to print
    SetPrintData,
    
    // get the current value of a setting
    GetSetting,
    
    // set a new value for a setting
    SetSetting,
    
    // restore the default value of a setting
    RestoreSetting,
    
    // upload the printers logs
    GetLogs,
    
    // download new printer firmware
    SetFirmware,
    
    // Show a test pattern
    Test,
    
    // Quit this application
    Exit
};

// ABC defining the interface to a class that handles commands.
class ICommandTarget
{
public:
    virtual void Handle(Command command) = 0;
    virtual void HandleError(const char* baseMsg, bool fatal = false, 
                             const char* str = NULL, int value = INT_MAX) = 0;
};

#endif	/* COMMANDS_H */

