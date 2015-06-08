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
    CPPUNIT_TEST(testValidate);
    CPPUNIT_TEST(testSetMaxJerk);
    CPPUNIT_TEST(testSetSpeed);
    CPPUNIT_TEST(testSetMicrosteppingMode);
    CPPUNIT_TEST(testSetUnitsPerRevolution);
    CPPUNIT_TEST(testSetStepAngle);
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

        CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetMicrosteppingMode(6);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400.0, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetUnitsPerRevolution(2);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3200.0, settings.PulsesPerUnit(), 1e-6);
        
        settings.SetStepAngle(900);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400.0, settings.PulsesPerUnit(), 1e-6);
    }

    void testValidate()
    {
        AxisSettings settings;

        CPPUNIT_ASSERT(settings.Validate() != MC_STATUS_SUCCESS);
        
        settings.SetStepAngle(1800);
        CPPUNIT_ASSERT(settings.Validate() != MC_STATUS_SUCCESS);
        
        settings.SetUnitsPerRevolution(1);
        CPPUNIT_ASSERT(settings.Validate() != MC_STATUS_SUCCESS);
        
        settings.SetMicrosteppingMode(1);
        CPPUNIT_ASSERT(settings.Validate() != MC_STATUS_SUCCESS);
        
        settings.SetMaxJerk(100);
        CPPUNIT_ASSERT(settings.Validate() != MC_STATUS_SUCCESS);
        
        settings.SetSpeed(100);
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.Validate());
    }

    void testSetMaxJerk()
    {
        AxisSettings settings;
        
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_MAX_JERK_SETTING_INVALID), settings.SetMaxJerk(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_MAX_JERK_SETTING_INVALID), settings.SetMaxJerk(-1));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.SetMaxJerk(100));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(100000000.0, settings.MaxJerk(), 1e-6);
    }
    
    void testSetSpeed()
    {
        AxisSettings settings;
        
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SPEED_SETTING_INVALID), settings.SetSpeed(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SPEED_SETTING_INVALID), settings.SetSpeed(-1));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.SetSpeed(100));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, settings.Speed(), 1e-6);
    }
    
    void testSetMicrosteppingMode()
    {
        AxisSettings settings;
        
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID), settings.SetMicrosteppingMode(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID), settings.SetMicrosteppingMode(7));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.SetMicrosteppingMode(1));
    }
    
    void testSetUnitsPerRevolution()
    {
        AxisSettings settings;
        
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID), settings.SetUnitsPerRevolution(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID), settings.SetUnitsPerRevolution(-1));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.SetUnitsPerRevolution(1));
    }
    
    void testSetStepAngle()
    {
        AxisSettings settings;
        
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_STEP_ANGLE_SETTING_INVALID), settings.SetStepAngle(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_STEP_ANGLE_SETTING_INVALID), settings.SetStepAngle(-1));
        CPPUNIT_ASSERT_EQUAL(static_cast<uint8_t>(MC_STATUS_SUCCESS), settings.SetStepAngle(1));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(AxisSettingsTest);
