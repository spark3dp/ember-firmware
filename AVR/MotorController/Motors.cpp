/*
 * Motors.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-28
 * Description: Motor driver control functions
 *              Freestanding functions are used rather than a general "motor" class with I/O register
 *              variables as members to avoid indirection when manipulating I/O registers
 */

#include <float.h>

#include "Motors.h"
#include "Hardware.h"

#ifdef DEBUG
#include "Debug.h"
#endif

extern "C"
{
#include "tinyg.h"
#include "util.h"
#include "canonical_machine.h"
#include "stepper.h"
#include "planner.h"
#include "kinematics.h"
}

/*
 * Initialize motor related I/O and subsystems
 */

void Motors::Initialize()
{
    // Initialize stepper motor control system
    st_init();

    // Initialize planning buffers
    mp_init();

    // Initialize canonical machine
    cm_init();

    // Set data direction for motor I/O pins
    MOTOR_SLEEP_DDR       |= MOTOR_SLEEP_DD_BM;
    MOTOR_ENABLE_DDR      |= MOTOR_ENABLE_DD_BM;
    MOTOR_RESET_DDR       |= MOTOR_RESET_DD_BM;
    MOTOR_MODE0_DDR       |= MOTOR_MODE0_DD_BM;
    MOTOR_MODE1_DDR       |= MOTOR_MODE1_DD_BM;
    MOTOR_MODE2_DDR       |= MOTOR_MODE2_DD_BM;
    MOTOR_Z_STEP_DDR      |= MOTOR_Z_STEP_DD_BM;
    MOTOR_Z_DIRECTION_DDR |= MOTOR_Z_DIRECTION_DD_BM;
    MOTOR_R_STEP_DDR      |= MOTOR_R_STEP_DD_BM;
    MOTOR_R_DIRECTION_DDR |= MOTOR_R_DIRECTION_DD_BM;
  
    // From DRV8825 data sheet:
    // SLEEP needs to be driven high for device operation
    MOTOR_SLEEP_PORT |= MOTOR_SLEEP_BM;

    // Disable drivers
    Disable();
}

/*
 * Reset the driver chips
 * Both chips are tied to the same reset signal
 */

void Motors::Reset()
{
    // From DRV8825 datasheet:
    // When the reset pin is driven low, the internal logic is reset and
    // the step table is reset to the home position

    MOTOR_RESET_PORT &= ~MOTOR_RESET_BM;
    MOTOR_RESET_PORT |= MOTOR_RESET_BM;
}

/*
 * Set the microstepping mode
 * Mode signals from each chip are tied together
 * modeFlag A flag determining the microstepping mode to use
 *          1 = full step, 2 = half step, ... 6 = 1/32 step
 */

void Motors::SetMicrosteppingMode(uint8_t modeFlag)
{
    switch(modeFlag)
    {
        case 1:
            // Full step mode
            MOTOR_MODE0_PORT &= ~MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT &= ~MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT &= ~MOTOR_MODE2_BM;
            break;
        case 2:
            // Half step mode
            MOTOR_MODE0_PORT |= MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT &= ~MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT &= ~MOTOR_MODE2_BM;
            break;
        case 3:
            // 1/4 step mode
            MOTOR_MODE0_PORT &= ~MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT |= MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT &= ~MOTOR_MODE2_BM;
            break;
        case 4:
            // 1/8 step mode
            MOTOR_MODE0_PORT |= MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT |= MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT &= ~MOTOR_MODE2_BM;
            break;
        case 5:
            // 1/16 step mode
            MOTOR_MODE0_PORT &= ~MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT &= ~MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT |= MOTOR_MODE2_BM;
            break;
        case 6:
            // 1/32 step mode
            MOTOR_MODE0_PORT |= MOTOR_MODE0_BM;
            MOTOR_MODE1_PORT &= ~MOTOR_MODE1_BM;
            MOTOR_MODE2_PORT |= MOTOR_MODE2_BM;
            break;
        default:
            //TODO: set error
            break;
    }
}

/*
 * Enable the motor drivers
 * Both chips are tied to the same enable signal
 */

void Motors::Enable()
{

    // From DRV8825 datasheet:
    // When ENABLE is driven low, device is enabled
    MOTOR_ENABLE_PORT &= ~MOTOR_ENABLE_BM;
}

/*
 * Disable the motor drivers
 * Both chips are tied to the same enable signal
 */

void Motors::Disable()
{
    MOTOR_ENABLE_PORT |= MOTOR_ENABLE_BM;
}

/*
 * Enqueue a movement block into the planning buffer
 * motorIndex The index corresponding to the motor to move
 * distance The distance to move
 * settings The settings for the axis to move
 */
void Motors::Move(uint8_t motorIndex, int32_t integralDistance, const AxisSettings& settings)
{
    PulsesPerUnit = settings.PulsesPerUnit();
    MaxJerk = settings.MaxJerk();
#ifdef DEBUG
    printf_P(PSTR("DEBUG: in Motors::Move, motor index: %d, distance: %ld, pulses per unit: %f, max jerk: %.0f\n"),
            motorIndex, integralDistance, static_cast<double>(PulsesPerUnit), static_cast<double>(MaxJerk));
#endif
    float minTime = 0;
    float distance = static_cast<float>(integralDistance);
    cm_cycle_start();
    mp_aline(distance, GetMoveTimes(&minTime, distance, settings.Speed(), settings.MaxSpeed()), 0 /* offset */, minTime);
}

/*
 * Compute minimum and optimal move times
 * minTime (output) The minimum time the move can take
 * distance The distance to move in units
 * speed The target speed at which to move
 * maxSpeed The maximum allowed speed
 * TODO: add a test
 */

float Motors::GetMoveTimes(float* minTime, float distance, float speed, float maxSpeed)
{
    float time = 0;      // coordinated move linear part at regular velocity
    float tempTime = 0;  // used in computation
    float maxTime = 0;  // time required for the rate-limiting axis
    *minTime = FLT_MAX; // arbitrarily large number

    time = distance / speed;
    tempTime = distance / maxSpeed;
    maxTime = max(maxTime, tempTime);
    *minTime = min(*minTime, tempTime);

    return (max(maxTime, time));
}
