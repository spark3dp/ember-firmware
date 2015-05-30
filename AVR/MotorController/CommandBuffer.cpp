/*
 * CommandBuffer.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-22
 * Description: FIFO style command queue implemented with a ring buffer
 */

#include <string.h>

#include "CommandBuffer.h"

CommandBuffer::CommandBuffer()
{
}

CommandBuffer::~CommandBuffer()
{
}

/*
 * Return whether or not the buffer contains any complete commands
 */

bool CommandBuffer::IsEmpty()
{
    return receivedCommandCount == 0;
}

/*
 * Add a single byte to the front of the buffer
 * Returns an error if the buffer is full or success if the buffer accepts the byte
 * byte The byte to add to the buffer
 */

Status CommandBuffer::AddByte(unsigned char byte)
{
    // Check if the buffer has room for an entire command
    // There may be space for a single byte but the buffer can only
    // accept the byte if capacity exists for the entire command

    if (receivedCommandCount == commandCapacity) return MC_STATUS_COMMAND_BUFFER_FULL;

    uint8_t nextHead = (head + 1) % COMMAND_BUFFER_SIZE;

    buffer[head] = byte;
    head = nextHead;

    if (--bytesRemaining == 0)
    {
        receivedCommandCount++;
        bytesRemaining = COMMAND_SIZE;
    }

    return MC_STATUS_SUCCESS;
}

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
 * Remove and return a byte from the front of the buffer
 */

unsigned char CommandBuffer::RemoveLastByte()
{
    uint8_t previousHead = (head - 1) % COMMAND_BUFFER_SIZE;
    unsigned char byte = buffer[previousHead];
    head = previousHead;
    bytesRemaining++;
    return byte;
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
