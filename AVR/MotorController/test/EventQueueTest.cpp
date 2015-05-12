/*
 * EventQueueTest.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-09
 */

#include <cppunit/extensions/HelperMacros.h>

#include "EventQueue.h"

class EventQueueTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(EventQueueTest);
    CPPUNIT_TEST(testIsEmpty);
    CPPUNIT_TEST(testQueue);
    CPPUNIT_TEST_SUITE_END();

private:

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testIsEmpty()
    {
        EventQueue eventQueue;
        EventData eventData;
        uint8_t eventCode = 0;
        
        CPPUNIT_ASSERT(eventQueue.IsEmpty());

        eventQueue.Add(eventCode, eventData);
        CPPUNIT_ASSERT(!eventQueue.IsEmpty());

        eventQueue.Remove(eventCode, eventData);
        CPPUNIT_ASSERT(eventQueue.IsEmpty());
    }

    void testQueue()
    {
        EventQueue eventQueue;
        int halfCapacity = (EVENT_QUEUE_LENGTH / 2) + 1;

        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            eventData.command = i;
            eventData.parameter = i;
            eventQueue.Add(i, eventData);
        }

        for (int i = 0; i < halfCapacity; i++)
        {
            uint8_t eventCode;
            EventData eventData;
            eventQueue.Remove(eventCode, eventData);
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventCode));
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventData.command));
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventData.parameter));
        }

        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            eventData.command = i + 50;
            eventData.parameter = i + 50;
            eventQueue.Add(i + 50, eventData);
        }

        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            uint8_t eventCode;
            eventQueue.Remove(eventCode, eventData);
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventCode));
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventData.command));
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventData.parameter));
        }

    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(EventQueueTest);

