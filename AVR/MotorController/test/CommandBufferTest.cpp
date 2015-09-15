//  File: CommandBufferTest.cpp
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

#include <cppunit/extensions/HelperMacros.h>

#include "CommandBuffer.h"
#include "Command.h"
#include "Status.h"

class CommandBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CommandBufferTest);
    CPPUNIT_TEST(testIsEmpty);
    CPPUNIT_TEST(testAddAndRemoveNonGeneralCommand);
    CPPUNIT_TEST(testAddAndRemoveGeneralCommand);
    CPPUNIT_TEST(testAddStatusRegister);
    CPPUNIT_TEST(testAddWhenCapacityExceeded);
    CPPUNIT_TEST(testIsFull);
    CPPUNIT_TEST_SUITE_END();

private:

    CommandBuffer* buffer;
    static const int TEST_COMMANDS_SIZE = 4;
    int32_t testCommandParameters[TEST_COMMANDS_SIZE] = { 1800, 500000, -60000, 48 };
    unsigned char nonGeneralCommands[TEST_COMMANDS_SIZE][COMMAND_SIZE] =
        {
            { MC_ROT_SETTINGS_REG, MC_STEP_ANGLE, 0x08, 0x07, 0x00, 0x00          }, // parameter value = 1800
            { MC_ROT_ACTION_REG,   MC_MOVE,       0x20, 0xA1, 0x07, 0x00          }, // parameter value = 500000
            { MC_Z_SETTINGS_REG ,  MC_JERK,       0xA0, 0x15, 0xFF, 0xFF          }, // parameter value = -60000
            { MC_Z_ACTION_REG,     MC_HOME,       MC_STATUS_REG, 0x00, 0x00, 0x00 }  // parameter value = 0
        };

private:

    void addNonGeneralCommand(int commandIndex)
    {
        for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
            buffer->AddCommandByte(nonGeneralCommands[commandIndex][byteIndex]);
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

        buffer->AddCommandByte(0x00);
        buffer->AddCommandByte(0x00);
        
        CPPUNIT_ASSERT(buffer->IsEmpty());
        
        buffer->AddCommandByte(0x00);
        buffer->AddCommandByte(0x00);
        buffer->AddCommandByte(0x00);
        buffer->AddCommandByte(0x00);
        
        CPPUNIT_ASSERT(!buffer->IsEmpty());

        Command command; 

        buffer->GetCommand(command);
        CPPUNIT_ASSERT(buffer->IsEmpty());
    }

    void testAddAndRemoveNonGeneralCommand()
    {
        Command command;

        // Write and read about half of the ring buffer capacity
        int commandCount = ((COMMAND_BUFFER_SIZE / COMMAND_SIZE) / 2) + 1;

        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            addNonGeneralCommand(commandIndex % TEST_COMMANDS_SIZE);
            
        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer->GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(testCommandParameters[exampleCommandIndex], command.Parameter());
        }

        // Write/read the same number of commands to test ring buffer wrap around
        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
            addNonGeneralCommand(commandIndex % TEST_COMMANDS_SIZE);

        for (int commandIndex = 0; commandIndex < commandCount; commandIndex++)
        {
            int exampleCommandIndex = commandIndex % 4;
            buffer->GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[exampleCommandIndex][0]), command.Register());
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[exampleCommandIndex][1]), command.Action());
            CPPUNIT_ASSERT_EQUAL(testCommandParameters[exampleCommandIndex], command.Parameter());
        }
    }

    void testAddAndRemoveGeneralCommand()
    {
        Command command;

        for (uint8_t i = MC_GENERAL_LOW_FENCEPOST + 1; i < MC_GENERAL_HIGH_FENCEPOST; i++)
            buffer->AddCommandByte(i);
        
        for (uint8_t i = MC_GENERAL_LOW_FENCEPOST + 1; i < MC_GENERAL_HIGH_FENCEPOST; i++)
        {
            buffer->GetCommand(command);
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(MC_GENERAL_REG), command.Register());
            CPPUNIT_ASSERT_EQUAL(i, command.Action());
            CPPUNIT_ASSERT_EQUAL(0, command.Parameter());
        }
    }

    void testAddStatusRegister()
    {
        Command command;
        
        buffer->AddCommandByte(MC_STATUS_REG);
        buffer->AddCommandByte(MC_RESET);
        
        buffer->GetCommand(command);
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(MC_GENERAL_REG), command.Register());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(MC_RESET), command.Action());
        CPPUNIT_ASSERT_EQUAL(0, command.Parameter());
    }

    void testAddWhenCapacityExceeded()
    {
        Command command;

        buffer->AddCommandByte(0x00);

        // Exceed capacity
        for (int i = 0; i < (COMMAND_BUFFER_SIZE / COMMAND_SIZE); i++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer->AddCommandByte(0x00);

        buffer->AddCommandByte(0x00);

        // The buffer stores bytes comprising entire commands until its capacity is reached
        for (int i = 0; i < COMMAND_BUFFER_SIZE / COMMAND_SIZE; i++)
            buffer->GetCommand(command);
        
        // The buffer contains no elements now
        // It correctly buffers additional data
        addNonGeneralCommand(0);
        buffer->GetCommand(command);

        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[0][0]), command.Register());
        CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(nonGeneralCommands[0][1]), command.Action());
        CPPUNIT_ASSERT_EQUAL(testCommandParameters[0], command.Parameter());
    }

    void testIsFull()
    {
        CPPUNIT_ASSERT(!buffer->IsFull());
        
        // Exceed capacity
        for (int i = 0; i < (COMMAND_BUFFER_SIZE / COMMAND_SIZE); i++)
            for (int byteIndex = 0; byteIndex < COMMAND_SIZE; byteIndex++)
                buffer->AddCommandByte(0x00);

        CPPUNIT_ASSERT(buffer->IsFull());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommandBufferTest);
