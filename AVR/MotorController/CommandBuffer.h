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
#include "Status.h"

class CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();

    void GetCommand(Command& command);
   
    /*
     * Make methods used in I2C ISR inline to eliminate overhead
     */

    /*
     * Return whether or not the buffer contains any complete commands
     */

    inline bool IsEmpty()
    {
        return receivedCommandCount == 0;
    }

    /*
     * Return whether or not the buffer can hold any more commands
     */

    inline bool IsFull()
    {
        return receivedCommandCount == commandCapacity;
    }

    /*
     * Add a single byte to the front of the buffer
     * Returns an error if the buffer is full or success if the buffer accepts the byte
     * byte The byte to add to the buffer
     */

    inline void AddByte(unsigned char byte)
    {
        // Check if the buffer has room for an entire command
        // There may be space for a single byte but the buffer can only
        // accept the byte if capacity exists for the entire command

        if (IsFull()) return;

        uint8_t nextHead = (head + 1) % COMMAND_BUFFER_SIZE;

        buffer[head] = byte;
        head = nextHead;

        if (--bytesRemaining == 0)
        {
            receivedCommandCount++;
            bytesRemaining = COMMAND_SIZE;
        }
    }

    /*
     * Remove and return a byte from the front of the buffer
     */

    inline unsigned char RemoveLastByte()
    {
        uint8_t previousHead = (head - 1) % COMMAND_BUFFER_SIZE;
        unsigned char byte = buffer[previousHead];
        head = previousHead;
        bytesRemaining++;
        return byte;
    }

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
    static uint8_t commandCapacity;
};

// Global instance externalized here for sharing between I2C module and main loop
extern CommandBuffer commandBuffer;

#endif /* COMMANDBUFFER_H */
