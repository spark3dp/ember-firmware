/*
 * CommandBuffer.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-22
 * Description: FIFO style command queue implemented with a ring buffer
 */

#include <string.h>

#include "CommandBuffer.h"

CommandBuffer::CommandBuffer() :
    head(0),
    tail(0),
    bytesRemaining(COMMAND_SIZE),
    receivedCommandCount(0)
{
}

CommandBuffer::~CommandBuffer()
{
}

/*
 * Return the number of complete commands in the buffer
 */

uint8_t CommandBuffer::ReceivedCommandCount()
{
    return receivedCommandCount;
}

/*
 * Add a single byte to the front of the buffer
 * byte The byte to add to the buffer
 */

void CommandBuffer::AddByte(unsigned char byte)
{
    uint8_t nextHead = (head + 1) % COMMAND_BUFFER_SIZE;

    if (nextHead != tail)
    {
        buffer[head] = byte;
        head = nextHead;

        if (--bytesRemaining == 0)
        {
            receivedCommandCount++;
            bytesRemaining = COMMAND_SIZE;
        }
    }
}

/*
 * Load the next command in the buffer into the specified command object
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
