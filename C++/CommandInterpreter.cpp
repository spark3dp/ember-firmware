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
#include <Error.h>

/// Public constructor, requires command target
CommandInterpreter::CommandInterpreter(ICommandTarget* target) :
_target(target)
{
    // create map of command strings to Command enum values
    _textCmdMap["START"] = Start;
    _textCmdMap["CANCEL"] = Cancel;
    _textCmdMap["PAUSE"] = Pause;
    _textCmdMap["RESUME"] = Resume;
    _textCmdMap["RESET"] = Reset;
    _textCmdMap["GETSTATUS"] = GetStatus;
    _textCmdMap["REFRESH"] = RefreshSettings;
    _textCmdMap["TEST"] = Test;
    _textCmdMap["GETFWVERSION"] = GetFWVersion;
    _textCmdMap["GETBOARDNUM"] = GetBoardNum;
    _textCmdMap["EXIT"] = Exit;
    _textCmdMap["QUIT"] = Exit;
    _textCmdMap["STARTPRINTDATALOAD"] = StartPrintDataLoad;
    _textCmdMap["PROCESSPRINTDATA"] = ProcessPrintData;
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
