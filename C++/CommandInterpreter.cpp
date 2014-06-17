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

#include <CommandInterpreter.h>
#include <Logger.h>
#include <Hardware.h>
#include <utils.h>

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
    _textCmdMap["TEST"] = Test;
    _textCmdMap["EXIT"] = Exit;
    _textCmdMap["QUIT"] = Exit;
}

/// Event handler callback
void CommandInterpreter::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case ButtonInterrupt:
           ButtonCallback((unsigned char*)data);
           break;
            
        case UICommand:
        case Keyboard:
            TextCommandCallback((char*)data);
            break;
            
        default:
            HandleImpossibleCase(eventType);
            break;
    } 
};

/// Translates button events from UI board into standard commands
void CommandInterpreter::ButtonCallback(unsigned char* status)
{ 
#ifdef DEBUG
//        std::cout << "button value = " << (int)*status  << std::endl;
#endif    

     Command cmd = UndefinedCommand;
    // translate the event to a command and pass it on to the print engine
    switch(*status)
    {
        case ERROR_STATUS:
            _target->HandleError(FRONT_PANEL_ERROR); 
            break;
            
        case BTN1_PRESS:                    
            // either start, pause, or resume a print
            cmd = StartPauseOrResume;
            break;
            
        case BTN2_PRESS:          
            // cancel the print
            cmd = Cancel;
            break;
            
        case BTNS_1_AND_2_PRESS: 
            cmd = Reset;
            break;
           
        // these cases not currently used
        case BTN1_HOLD:
        case BTN2_HOLD:
        case BTN3_PRESS:      
        case BTN3_HOLD:
            break;  // button 3 not currently used
            
        default:
            _target->HandleError(UNKNOWN_FRONT_PANEL_STATUS, false, NULL, 
                                                                (int)*status);
            break;
    }
    
    // if command successfully translated, handle it
    if(cmd != UndefinedCommand)
        _target->Handle(cmd);        
}



/// Translates UI text command input into standard commands and pass them on
/// to their handler
void CommandInterpreter::TextCommandCallback(char* cmd)
{  
#ifdef DEBUG
//    std::cout << "in CommandInterpreter::TextCommandCallback command = " << 
//                 cmd << std::endl;
#endif       
    
    // map command string to a command code
    Command command = (Command)_textCmdMap[std::string(CmdToUpper(cmd))];
    
    if(command == UndefinedCommand)
    {
        _target->HandleError(UNKNOWN_TEXT_COMMAND_ERROR, false, cmd);
    }
    else
    {
        // if command successfully translated, handle it
        _target->Handle(command);
    }
}
