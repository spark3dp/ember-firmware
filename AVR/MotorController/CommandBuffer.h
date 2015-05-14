/*
 * CommandBuffer.h
 * Author: Jason Lefley
 * Date  : 2015-04-22
 */

#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

// Number of bytes comprising a command
#define COMMAND_SIZE 6

// Size in bytes of ring buffer
// Needs to be a power of 2 so compiler can use AND to implement modulo
#define COMMAND_BUFFER_SIZE 128

#include <stdint.h>
#include "Command.h"

class CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    bool IsEmpty();
    void AddByte(unsigned char byte);
    void GetCommand(Command& command);
    unsigned char RemoveLastByte();

private:
    CommandBuffer(const CommandBuffer&);
    unsigned char RemoveByte();

private:
    // These variables are modified in both an ISR and in the main
    // so they need to be qualified with volatile
    volatile unsigned char buffer[COMMAND_BUFFER_SIZE];
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;
    volatile uint8_t bytesRemaining = COMMAND_SIZE;
    volatile uint8_t receivedCommandCount = 0;
    uint8_t commandCapacity = COMMAND_BUFFER_SIZE / COMMAND_SIZE;
};

#endif /* COMMANDBUFFER_H */
