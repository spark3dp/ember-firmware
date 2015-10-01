//  File:   CommandInterpreter.cpp
//  Interprets commands from various sources (buttons, web, USB, keyboard), 
//  translates them into standard printer commands, and forwards them to a 
//  target, e.g. the print engine. 
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
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

#include <iostream>
#include <algorithm>

#include <CommandInterpreter.h>
#include <Logger.h>
#include <Hardware.h>
#include <utils.h>
#include <ErrorMessage.h>
#include <Shared.h>

// Public constructor, requires command target
CommandInterpreter::CommandInterpreter(ICommandTarget* target) :
_target(target)
{
    // create map of command strings to Command enum values
    _textCmdMap[CMD_START_PRINT] = Start;
    _textCmdMap[CMD_CANCEL] = Cancel;
    _textCmdMap[CMD_PAUSE] = Pause;
    _textCmdMap[CMD_RESUME] = Resume;
    _textCmdMap[CMD_RESET_PRINTER] = Reset;
    _textCmdMap[CMD_REFRESH_SETTINGS] = RefreshSettings;
    _textCmdMap[CMD_TEST] = Test;
    _textCmdMap[CMD_CAL_IMAGE] = CalImage;
    _textCmdMap[CMD_EXIT] = Exit;
    _textCmdMap[CMD_QUIT] = Exit;
    _textCmdMap[CMD_SHOW_PRINT_DATA_DOWNLOADING] = ShowPrintDataDownloading;
    _textCmdMap[CMD_SHOW_PRINT_DOWNLOAD_FAILED] = ShowPrintDownloadFailed;
    _textCmdMap[CMD_START_PRINT_DATA_LOAD] = StartPrintDataLoad;
    _textCmdMap[CMD_PROCESS_PRINT_DATA] = ProcessPrintData;
    _textCmdMap[CMD_SHOW_PRINT_DATA_LOADED] = ShowPrintDataLoaded;
    _textCmdMap[CMD_REGISTRATION_CODE] = StartRegistering;
    _textCmdMap[CMD_REGISTERED] = RegistrationSucceeded;
    _textCmdMap[CMD_APPLY_SETTINGS] = ApplySettings;
    _textCmdMap[CMD_SHOW_WIRELESS_CONNECTING] = ShowWiFiConnecting;
    _textCmdMap[CMD_SHOW_WIRELESS_CONNECTION_FAILED] = ShowWiFiConnectionFailed;
    _textCmdMap[CMD_SHOW_WIRELESS_CONNECTED] = ShowWiFiConnected;
    _textCmdMap[CMD_DISMISS] = Dismiss;
    _textCmdMap[CMD_BTN1] = Button1;
    _textCmdMap[CMD_BTN2] = Button2;
    _textCmdMap[CMD_BTN1_HOLD] = Button1Hold;
    _textCmdMap[CMD_BTN2_HOLD] = Button2Hold;
    _textCmdMap[CMD_BTNS_1_AND_2] = Buttons1and2;
    _textCmdMap[CMD_BTNS_1_AND_2_HOLD] = Buttons1and2Hold;
}

// Event handler callback
void CommandInterpreter::Callback(EventType eventType, const EventData& data)
{
    switch(eventType)
    {            
        case UICommand:
        case Keyboard:
            TextCommandCallback(data.Get<std::string>());
            break;
            
        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), 
                            eventType);
            break;
    } 
};

// Translates UI text command input into standard commands and pass them on
// to their handler
void CommandInterpreter::TextCommandCallback(std::string cmd)
{      
    // convert the command string to upper case
    std::transform (cmd.begin(), cmd.end(), cmd.begin(), toupper);
    // remove whitespace and anything after it
    std::string::size_type p = cmd.find_first_of(" \t\n");
    if (p != std::string::npos)
        cmd.erase(p);

    // map command string to a command code
    Command command = (Command)_textCmdMap[cmd];
    
    if (command == UndefinedCommand)
    {
        _target->HandleError(UnknownTextCommand, false, cmd.c_str());
    }
    else
    {
        // if command successfully translated, handle it
        _target->Handle(command);
    }
}
