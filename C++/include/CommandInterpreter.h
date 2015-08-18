/* 
 * File:   CommandInterpreter.h
 * Author: Richard Greene
 * 
 * Interprets commands from various sources (buttons, web, USB, keyboard), 
 * translates them into standard printer commands, and forwards them to the 
 * print engine. 
 *
 * Created on May 21, 2014, 4:37 PM
 */

#ifndef COMMANDINTERPRETER_H
#define	COMMANDINTERPRETER_H

#include <string>
#include <map>

#include <Command.h>
#include "EventType.h"

class CommandInterpreter : public ICallback
{  
public:
    CommandInterpreter(ICommandTarget* target);
    virtual void Callback(EventType eventType, void* data);
    
protected:  
    // don't allow construction without a command target
    CommandInterpreter() {}
    
private:  
    ICommandTarget* _target;
    std::map<std::string, int> _textCmdMap;

    void TextCommandCallback(std::string cmd);
};

#endif	/* COMMANDINTERPRETER_H */

