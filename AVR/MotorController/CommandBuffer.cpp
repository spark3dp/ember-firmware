//  File: CommandBuffer.cpp
//  FIFO style command queue implemented with a ring buffer
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

#include "CommandBuffer.h"

CommandBuffer::CommandBuffer()
{
}

CommandBuffer::~CommandBuffer()
{
}

uint8_t CommandBuffer::commandCapacity = COMMAND_BUFFER_SIZE / COMMAND_SIZE;

// Remove the next command in the buffer and load it into the specified command object
// command The command to populate
void CommandBuffer::GetCommand(Command& command)
{
    if (receivedCommandCount == 0) return;

    command.SetRegister(RemoveByte());
    command.SetAction(RemoveByte());

    unsigned char parameterBytes[4];

    parameterBytes[0] = RemoveByte();
    parameterBytes[1] = RemoveByte();
    parameterBytes[2] = RemoveByte();
    parameterBytes[3] = RemoveByte();

    command.SetParameterBytes(parameterBytes);

    receivedCommandCount--;
}

// Remove and return a byte from the end of the buffer
unsigned char CommandBuffer::RemoveByte()
{
    unsigned char byte = buffer[tail];
    tail = (tail + 1) % COMMAND_BUFFER_SIZE;
    return byte;
}

