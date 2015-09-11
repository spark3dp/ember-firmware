//  File: Command.h
//  Encapsulates and provides access to a command's register, action, and parameter
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

class Command
{
    public:
        Command();
        ~Command();

        void SetRegister(unsigned char value);
        unsigned char Register() const;

        void SetAction(unsigned char value);
        unsigned char Action() const;

        void SetParameterBytes(const unsigned char* buffer);
        int32_t Parameter() const;

    private:
        Command(const Command&);
        
    private:
        unsigned char mRegister;
        unsigned char mAction;
        int32_t mParameter;
};

#endif  // COMMAND_H
