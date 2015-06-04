/*
 * CommandBufferTest.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-29
 */

#include <cppunit/extensions/HelperMacros.h>

#include "CommandBuffer.h"
#include "Command.h"
#include "Status.h"

class CommandBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CommandBufferTest);
    CPPUNIT_TEST(testIsEmpty);
    CPPUNIT_TEST(testAddAndRemoveCommand);
    CPPUNIT_TEST(testAddWhenCapacityExceeded);
    CPPUNIT_TEST(testRemoveLastByte);
    CPPUNIT_TEST(testIsFull);
    CPPUNIT_TEST_SUITE_END();

private:

    CommandBuffer* buffer;
    static const int TEST_COMMANDS_SIZE = 4;
    int32_t testCommandParameters[TEST_COMMANDS_SIZE] = { 1800, 500000, -60000, 0 };
    unsigned char testCommands[TEST_COMMANDS_SIZE][COMMAND_SIZE] =
        {
            { 0x22, 0x01, 0x08, 0x07, 0x00, 0x00 }, // parameter value = 1800
            { 0x21, 0x03, 0x20, 0xA1, 0x07, 0x00 }, // parameter value = 500000
            { 0x23, 0x05, 0xA0, 0x15, 0xFF, 0xFF }, // parameter value = -60000
            { 0x24, 0x04, 0x00, 0x00, 0x00, 0x00 }  // parameter value = 0
        };

private:

    void addTestCommand(int commandIndex)
    {
        for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
            buffer->AddByte(testCommands[commandIndex][byteIndex]);
    }

public:
    void setUp()
    {
        buffer = new CommandBuffer();
    }

    void tearDown()
    {
        delete buffer;
    }

    void testIsEmpty()
    {
        CPPUNIT_ASSERT(buffer->IsEmpty());

        buffer->AddByte(0x00);
        buffer->AddByte(0x00);
        
        CPPUNIT_ASSERT(buffer->IsEmpty());
        
        buffer->AddByte(0x00);
        buffer->AddByte(0x00);
        buffer->AddByte(0x00);
        buffer->AddByte(0x00);
        
        CPPUNIT_ASSERT(!buffer->IsEmpty());

        Command command; 

        buffer->GetCommand(command);
        CPPUNIT_ASSERT(buffer->IsEmpty());
    }

    void testAddAndRemoveCommand()
    {
        Command command;

        // Write and read about half of the ring buffer capacity
        int commandCount = ((COMMAND_BUFFER_SIZE / COMMAND_SIZE) / 2) + 1;

        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            addTestCommand(commandIndex % TEST_COMMANDS_SIZE);
            
        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer->GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(testCommandParameters[exampleCommandIndex], command.Parameter());
        }

        // Write/read the same number of commands to test ring buffer wrap around
        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            addTestCommand(commandIndex % TEST_COMMANDS_SIZE);

        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer->GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(testCommandParameters[exampleCommandIndex], command.Parameter());
        }
    }

    void testAddWhenCapacityExceeded()
    {
        Command command;

        buffer->AddByte(0x00);

        // Exceed capacity
        for (int i = 0; i < (COMMAND_BUFFER_SIZE / COMMAND_SIZE); i++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer->AddByte(0x00);

        buffer->AddByte(0x00);

        // The buffer stores bytes comprising entire commands until its capacity is reached
        for (int i = 0; i < COMMAND_BUFFER_SIZE / COMMAND_SIZE; i++)
            buffer->GetCommand(command);
        
        // The buffer contains no elements now
        // It correctly buffers additional data
        addTestCommand(0);
        buffer->GetCommand(command);

        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[0][0]), command.Register());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[0][1]), command.Action());
        CPPUNIT_ASSERT_EQUAL(testCommandParameters[0], command.Parameter());
    }

    void testRemoveLastByte()
    {
        Command command;

        // Add a stray byte, not part of a command
        buffer->AddByte(0x05);

        unsigned char lastByte = buffer->RemoveLastByte();
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0x05), lastByte);

        // Add a full command
        addTestCommand(0);
        
        buffer->GetCommand(command);

        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[0][0]), command.Register());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[0][1]), command.Action());
        CPPUNIT_ASSERT_EQUAL(testCommandParameters[0], command.Parameter());

        // Fill to capacity
        for (int i = 0; i < COMMAND_BUFFER_SIZE / COMMAND_SIZE; i++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer->AddByte(0x00);

        // Empty
        for (int i = 0; i < COMMAND_BUFFER_SIZE / COMMAND_SIZE; i++)
            buffer->GetCommand(command);

        // Add a stray byte, not part of a command
        buffer->AddByte(0x01);

        lastByte = buffer->RemoveLastByte();
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0x01), lastByte);

        // Add a full command
        addTestCommand(2);

        buffer->GetCommand(command);

        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[2][0]), command.Register());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(testCommands[2][1]), command.Action());
        CPPUNIT_ASSERT_EQUAL(testCommandParameters[2], command.Parameter());
    }

    void testIsFull()
    {
        CPPUNIT_ASSERT(!buffer->IsFull());
        
        // Exceed capacity
        for (int i = 0; i < (COMMAND_BUFFER_SIZE / COMMAND_SIZE); i++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer->AddByte(0x00);

        CPPUNIT_ASSERT(buffer->IsFull());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommandBufferTest);
