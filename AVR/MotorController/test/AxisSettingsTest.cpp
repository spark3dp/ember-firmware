/*
 * AxisSettingsTest.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-29
 */

#include <cppunit/extensions/HelperMacros.h>

#include "AxisSettings.h"

class AxisSettingsTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(AxisSettingsTest);
    CPPUNIT_TEST(testStepsPerUnit);
    CPPUNIT_TEST_SUITE_END();

private:


public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testStepsPerUnit()
    {
        AxisSettings settings;

        settings.SetStepAngle(1800);
        settings.SetUnitsPerRevolution(1);
        settings.SetMicrosteppingMode(1);

        CPPUNIT_ASSERT_DOUBLES_EQUAL(200, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetMicrosteppingMode(6);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetUnitsPerRevolution(2);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3200, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetStepAngle(900);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400, settings.PulsesPerUnit(), 1e-6);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(AxisSettingsTest);
