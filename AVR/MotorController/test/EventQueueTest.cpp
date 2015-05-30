/*
 * EventQueueTest.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-09
 */

#include <cppunit/extensions/HelperMacros.h>

#include "EventQueue.h"
#include "Status.h"

class EventQueueTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(EventQueueTest);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testQueue);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST_SUITE_END();

private:

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testAdd()
    {
        EventQueue eventQueue;
        EventData eventData;

        CPPUNIT_ASSERT_EQUAL(eventQueue.Add(0, eventData), static_cast<uint8_t>(MC_STATUS_SUCCESS));

        for (int i = 0; i < EVENT_QUEUE_LENGTH; i++)
            eventQueue.Add(0, eventData);
        
        CPPUNIT_ASSERT_EQUAL(eventQueue.Add(0, eventData), static_cast<uint8_t>(MC_STATUS_EVENT_QUEUE_FULL));
    }

    void testQueue()
    {
        EventQueue eventQueue;
        int halfCapacity = (EVENT_QUEUE_LENGTH / 2) + 1;

        CPPUNIT_ASSERT(eventQueue.IsEmpty());
        
        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            eventData.command = i;
            eventData.parameter = i;
            eventQueue.Add(i, eventData);
        }

        CPPUNIT_ASSERT(!eventQueue.IsEmpty());
        
        for (int i = 0; i < halfCapacity; i++)
        {
            uint8_t eventCode;
            EventData eventData;
            eventQueue.Remove(eventCode, eventData);
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventCode));
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventData.command));
            CPPUNIT_ASSERT_EQUAL(i, static_cast<int>(eventData.parameter));
        }
        
        CPPUNIT_ASSERT(eventQueue.IsEmpty());

        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            eventData.command = i + 50;
            eventData.parameter = i + 50;
            eventQueue.Add(i + 50, eventData);
        }

        CPPUNIT_ASSERT(!eventQueue.IsEmpty());

        for (int i = 0; i < halfCapacity; i++)
        {
            EventData eventData;
            uint8_t eventCode;
            eventQueue.Remove(eventCode, eventData);
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventCode));
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventData.command));
            CPPUNIT_ASSERT_EQUAL(i + 50, static_cast<int>(eventData.parameter));
        }
        
        CPPUNIT_ASSERT(eventQueue.IsEmpty());

    }

    void testClear()
    {
        EventQueue eventQueue;
        EventData eventData;
        eventQueue.Add(0, eventData);
        eventQueue.Clear();
        CPPUNIT_ASSERT(eventQueue.IsEmpty());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(EventQueueTest);

