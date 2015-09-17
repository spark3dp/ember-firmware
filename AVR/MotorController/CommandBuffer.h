//  File: CommandBuffer.h
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

#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

// Number of bytes comprising a command
#define COMMAND_SIZE 6

// Size in bytes of ring buffer
// Needs to be a power of 2 so compiler can use AND to implement modulo
#define COMMAND_BUFFER_SIZE 128

#include <stdint.h>
#include "Command.h"
#include "Status.h"

class CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    void GetCommand(Command& command);
   
    // Make methods used in I2C ISR inline to eliminate overhead

    // Return whether or not the buffer contains any complete commands
    inline bool IsEmpty()
    {
        return receivedCommandCount == 0;
    }

    // Return whether or not the buffer can hold any more commands
    inline bool IsFull()
    {
        return receivedCommandCount == commandCapacity;
    }

    // Handle adding a byte or bytes to the buffer
    //
    // If the specified data is a general command and the buffer is not in the
    // process of receiving a multi-byte command, add appropriate register and
    // parameter values for consistency to represent the general command
    //
    // If the specified data represents a read register and the buffer is not
    // in the process of receiving a multi-byte command, don't add the data
    //
    // Otherwise, assume the data is part of a command transmitted in COMMAND_SIZE
    // bytes
    // 
    // data The byte to conditionally add to the buffer
    inline void AddCommandByte(unsigned char data)
    {
        if (data == MC_STATUS_REG && bytesRemaining == COMMAND_SIZE)
            // The controller received a read register address
            // Ignore as the data does not represent a command
            return;

        if (data > MC_GENERAL_LOW_FENCEPOST && data < MC_GENERAL_HIGH_FENCEPOST && bytesRemaining == COMMAND_SIZE)
        {
            // data represents a general command
            AddByte(MC_GENERAL_REG);
            AddByte(data);
            AddByte(0x00);
            AddByte(0x00);
            AddByte(0x00);
            AddByte(0x00);
        }
        else
            // data is part of a multi-byte command
            AddByte(data);
    }

private:
    CommandBuffer(const CommandBuffer&);
    unsigned char RemoveByte();

    // Add a single byte to the front of the buffer if the buffer has capacity
    // for an entire 6-byte command
    // data The byte to add to the buffer
    inline void AddByte(unsigned char data)
    {
        // Check if the buffer has room for an entire command
        // There may be space for a single byte but the buffer can only
        // accept the data if capacity exists for the entire command

        if (IsFull()) return;

        uint8_t nextHead = (head + 1) % COMMAND_BUFFER_SIZE;

        buffer[head] = data;
        head = nextHead;

        if (--bytesRemaining == 0)
        {
            receivedCommandCount++;
            bytesRemaining = COMMAND_SIZE;
        }
    }

private:
    // These variables are modified in both an ISR and in the main
    // so they need to be qualified with volatile
    volatile unsigned char buffer[COMMAND_BUFFER_SIZE];
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;
    volatile uint8_t bytesRemaining = COMMAND_SIZE;
    volatile uint8_t receivedCommandCount = 0;
    static uint8_t commandCapacity;
};

// Global instance externalized here for sharing between I2C module and main loop
extern CommandBuffer commandBuffer;

#endif  // COMMANDBUFFER_H
