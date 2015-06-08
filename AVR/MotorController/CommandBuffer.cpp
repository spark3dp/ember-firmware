/*
 * CommandBuffer.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-22
 * Description: FIFO style command queue implemented with a ring buffer
 */

#include "CommandBuffer.h"

CommandBuffer::CommandBuffer()
{
}

CommandBuffer::~CommandBuffer()
{
}

uint8_t CommandBuffer::commandCapacity = COMMAND_BUFFER_SIZE / COMMAND_SIZE;

/*
 * Remove the next command in the buffer and load it into the specified command object
 * command The command to populate
 */

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

/*
 * Remove and return a byte from the end of the buffer
 */

unsigned char CommandBuffer::RemoveByte()
{
    unsigned char byte = buffer[tail];
    tail = (tail + 1) % COMMAND_BUFFER_SIZE;
    return byte;
}

