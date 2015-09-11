//  File: Command.cpp
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

#include "Command.h"


Command::Command() :
    mRegister(0),
    mAction(0),
    mParameter(0)
{
}

Command::~Command()
{
}

// Set the command's register value
// value The register value
void Command::SetRegister(unsigned char value)
{
    mRegister = value;
}

// Return the command's register value
unsigned char Command::Register() const
{
    return mRegister;
}

// Set the command's action value
// value The action value
void Command::SetAction(unsigned char value)
{
    mAction = value;
}

// Return the command's action value
unsigned char Command::Action() const
{
    return mAction;
}

// Sets the command's parameter by assembling the comprising bytes
// buffer A byte length of 4 containing the parameter bytes (LSB first)
void Command::SetParameterBytes(const unsigned char* buffer)
{
    int32_t ones = 0xFFFFFFFF;

    mParameter = (buffer[0] & ones) | (buffer[1] & ones) << 8 |
        (buffer[2] & ones) << 16 | (buffer[3] & ones) << 24;
}

// Return the command's parameter value
int32_t Command::Parameter() const
{
    return mParameter;
}

