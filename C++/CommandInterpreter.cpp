/* 
 * File:   CommandInterpreter.cpp
 * Author: Richard Greene
 * 
 * Interprets commands from various sources (buttons, web, USB, keyboard), 
 * translates them into standard printer commands, and forwards them to the 
 * print engine. 
 *
 * Created on May 21, 2014, 4:37 PM
 */

#include <iostream>
#include <algorithm>

#include <CommandInterpreter.h>
#include <Logger.h>
#include <Hardware.h>
#include <utils.h>
#include <ErrorMessage.h>
#include <Shared.h>

/// Public constructor, requires command target
CommandInterpreter::CommandInterpreter(ICommandTarget* target) :
_target(target)
{
    // create map of command strings to Command enum values
    _textCmdMap[CMD_START_PRINT] = Start;
    _textCmdMap[CMD_CANCEL] = Cancel;
    _textCmdMap[CMD_PAUSE] = Pause;
    _textCmdMap[CMD_RESUME] = Resume;
    _textCmdMap[CMD_RESET_PRINTER] = Reset;
    _textCmdMap[CMD_GET_STATUS] = GetStatus;
    _textCmdMap[CMD_REFRESH_SETTINGS] = RefreshSettings;
    _textCmdMap[CMD_TEST] = Test;
    _textCmdMap[CMD_CAL_IMAGE] = CalImage;
    _textCmdMap[CMD_GET_FW_VERSION] = GetFWVersion;
    _textCmdMap[CMD_GET_BOARD_NUM] = GetBoardNum;
    _textCmdMap[CMD_EXIT] = Exit;
    _textCmdMap[CMD_QUIT] = Exit;
    _textCmdMap[CMD_SHOW_PRINT_DATA_DOWNLOADING] = ShowPrintDataDownloading;
    _textCmdMap[CMD_START_PRINT_DATA_LOAD] = StartPrintDataLoad;
    _textCmdMap[CMD_PROCESS_PRINT_DATA] = ProcessPrintData;
    _textCmdMap[CMD_SHOW_PRINT_DATA_LOADED] = ShowPrintDataLoaded;
    _textCmdMap[CMD_REGISTRATION_CODE] = StartRegistering;
    _textCmdMap[CMD_REGISTERED] = RegistrationSucceeded;
    _textCmdMap[CMD_APPLY_PRINT_SETTINGS] = ApplyPrintSettings;
    _textCmdMap[CMD_CALIBRATE] = StartCalibration;
}

/// Event handler callback
void CommandInterpreter::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {            
        case UICommand:
        case Keyboard:
            TextCommandCallback((char*)data);
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    } 
};

/// Translates UI text command input into standard commands and pass them on
/// to their handler
void CommandInterpreter::TextCommandCallback(std::string cmd)
{  
#ifdef DEBUG
//    std::cout << "in CommandInterpreter::TextCommandCallback command = " << 
//                 cmd << std::endl;
#endif       
    
    // convert the command string to upper case
    std::transform (cmd.begin(), cmd.end(), cmd.begin(), toupper);
    // remove whitespace and anything after it
    std::string::size_type p = cmd.find_first_of(" \t\n");
    if(p != std::string::npos)
        cmd.erase(p);

    // map command string to a command code
    Command command = (Command)_textCmdMap[cmd];
    
    if(command == UndefinedCommand)
    {
        _target->HandleError(UnknownTextCommand, false, cmd.c_str());
    }
    else
    {
        // if command successfully translated, handle it
        _target->Handle(command);
    }
}
