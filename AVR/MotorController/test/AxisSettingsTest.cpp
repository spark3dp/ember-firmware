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
    CPPUNIT_TEST(testValidateWhenAllSettingsAreValid);
    CPPUNIT_TEST(testValidateWhenMaxJerkIsZero);
    CPPUNIT_TEST(testValidateWhenMaxJerkIsNegative);
    CPPUNIT_TEST(testValidateWhenSpeedIsZero);
    CPPUNIT_TEST(testValidateWhenSpeedIsNegative);
    CPPUNIT_TEST(testValidateWhenMicrosteppingModeIsNegative);
    CPPUNIT_TEST(testValidateWhenMicrosteppingModeIsZero);
    CPPUNIT_TEST(testValidateWhenMicrosteppingModeExceedsMaximum);
    CPPUNIT_TEST(testValidateWhenUnitsPerRevolutionIsZero);
    CPPUNIT_TEST(testValidateWhenUnitsPerRevolutionIsNegative);
    CPPUNIT_TEST(testValidateWhenStepAngleIsZero);
    CPPUNIT_TEST(testValidateWhenStepAngleIsNegative);
    CPPUNIT_TEST_SUITE_END();

private:
    AxisSettings* settings;


public:
    void setUp()
    {
        settings = new AxisSettings();
        settings->SetStepAngle(1800);
        settings->SetUnitsPerRevolution(1);
        settings->SetMaxJerk(100);
        settings->SetSpeed(100);
        settings->SetMicrosteppingMode(1);
    }

    void tearDown()
    {
        delete settings;
    }

    void testStepsPerUnit()
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(200, settings->PulsesPerUnit(), 1e-6);
        
        settings->SetMicrosteppingMode(6);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400, settings->PulsesPerUnit(), 1e-6);
        
        settings->SetUnitsPerRevolution(2);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3200, settings->PulsesPerUnit(), 1e-6);
        
        settings->SetStepAngle(900);
        
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6400, settings->PulsesPerUnit(), 1e-6);
    }

    void testValidateWhenAllSettingsAreValid()
    {
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_SUCCESS));
    }

    void testValidateWhenMaxJerkIsZero()
    {
        settings->SetMaxJerk(0);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_MAX_JERK_SETTING_INVALID));
    }
    
   void testValidateWhenMaxJerkIsNegative()
   {
        settings->SetMaxJerk(-1);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_MAX_JERK_SETTING_INVALID));
    }

    void testValidateWhenSpeedIsZero()
    {
        settings->SetSpeed(0);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_SPEED_SETTING_INVALID));
    }
    
    void testValidateWhenSpeedIsNegative()
    {
        settings->SetSpeed(-1);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_SPEED_SETTING_INVALID));
    }

    void testValidateWhenMicrosteppingModeIsZero()
    {
        settings->SetMicrosteppingMode(0);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID));
    }
    
    void testValidateWhenMicrosteppingModeExceedsMaximum()
    {
        settings->SetMicrosteppingMode(7);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID));
    }
    
    void testValidateWhenMicrosteppingModeIsNegative()
    {
        settings->SetMicrosteppingMode(-1);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_MICROSTEPPING_MODE_SETTING_INVALID));
    }

    void testValidateWhenUnitsPerRevolutionIsZero()
    {
        settings->SetUnitsPerRevolution(0);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID));
    }
    
    void testValidateWhenUnitsPerRevolutionIsNegative()
    {
        settings->SetUnitsPerRevolution(-1);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_UNITS_PER_REVOLUTION_SETTING_INVALID));
    }

    void testValidateWhenStepAngleIsZero()
    {
        settings->SetStepAngle(0);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_STEP_ANGLE_SETTING_INVALID));
    }
    
    void testValidateWhenStepAngleIsNegative()
    {
        settings->SetStepAngle(-1);
        CPPUNIT_ASSERT_EQUAL(settings->Validate(), static_cast<uint8_t>(MC_STATUS_STEP_ANGLE_SETTING_INVALID));
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(AxisSettingsTest);
