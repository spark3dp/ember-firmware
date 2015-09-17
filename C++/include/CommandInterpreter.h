//  File:   CommandInterpreter.h
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
    virtual void Callback(EventType eventType, const EventData& data);
        
private:  
    ICommandTarget* _target;
    std::map<std::string, int> _textCmdMap;

    void TextCommandCallback(std::string cmd);
};

#endif    // COMMANDINTERPRETER_H

