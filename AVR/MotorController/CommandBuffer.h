/*
 * CommandBuffer.h
 * Author: Jason Lefley
 * Date  : 2015-04-22
 */

#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#define COMMAND_SIZE 6          // Number of bytes comprising a command
#define COMMAND_BUFFER_SIZE 128 // Size in bytes of ring buffer

#include <stdint.h>
#include "Command.h"

class CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    uint8_t ReceivedCommandCount();
    void AddByte(unsigned char byte);
    void GetCommand(Command& command);

private:
    CommandBuffer(const CommandBuffer&);
    unsigned char RemoveByte();

private:
    // These variables are modified in both an ISR and in the main
    // so they need to be qualified with volatile
    volatile unsigned char buffer[COMMAND_BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t bytesRemaining;
    volatile uint8_t receivedCommandCount;
};

#endif /* COMMANDBUFFER_H */
