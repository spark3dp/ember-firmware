/*
 * CommandBufferTest.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-29
 */

#include <cppunit/extensions/HelperMacros.h>

#include "CommandBuffer.h"
#include "Command.h"

class CommandBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CommandBufferTest);
    CPPUNIT_TEST(testReceivedCommandCount);
    CPPUNIT_TEST(testAddAndRemoveCommand);
    CPPUNIT_TEST_SUITE_END();

private:


public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testReceivedCommandCount()
    {
        CommandBuffer buffer;

        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(buffer.ReceivedCommandCount()));

        buffer.AddByte(0x00);
        buffer.AddByte(0x00);
        
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(buffer.ReceivedCommandCount()));
        
        buffer.AddByte(0x00);
        buffer.AddByte(0x00);
        buffer.AddByte(0x00);
        buffer.AddByte(0x00);
        
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(buffer.ReceivedCommandCount()));

        for (int i = 0; i < COMMAND_SIZE; i++)
            buffer.AddByte(0x00);
        
        CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(buffer.ReceivedCommandCount()));
       
        Command command; 

        buffer.GetCommand(command);
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(buffer.ReceivedCommandCount()));

        buffer.GetCommand(command);
        CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(buffer.ReceivedCommandCount()));
    }

    void testAddAndRemoveCommand()
    {
        unsigned char commands[4][COMMAND_SIZE] =
        {
            { 0x22, 0x01, 0x08, 0x07, 0x00, 0x00 }, // parameter value = 1800
            { 0x21, 0x03, 0x20, 0xA1, 0x07, 0x00 }, // parameter value = 500000
            { 0x23, 0x05, 0xA0, 0x15, 0xFF, 0xFF }, // parameter value = -60000
            { 0x24, 0x04, 0x00, 0x00, 0x00, 0x00 }  // parameter value = 0
        };

        int32_t expectedParameters[4] = { 1800, 500000, -60000, 0 };

        CommandBuffer buffer;
        Command command;

        // Write and read about half of the ring buffer capacity
        int commandCount = ((COMMAND_BUFFER_SIZE / COMMAND_SIZE) / 2) + 1;

        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer.AddByte(commands[commandIndex % 4][byteIndex]);


        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer.GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(commands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(commands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(expectedParameters[exampleCommandIndex], command.Parameter());
        }

        // Write/read the same number of commands to test ring buffer wrap around
        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer.AddByte(commands[commandIndex % 4][byteIndex]);


        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer.GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(commands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(commands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(expectedParameters[exampleCommandIndex], command.Parameter());
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommandBufferTest);
