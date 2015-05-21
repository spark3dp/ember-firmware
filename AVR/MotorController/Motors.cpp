/*
 * Motors.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-28
 * Description: Motor driver control functions
 *              Freestanding functions are used rather than a general "motor" class with I/O register
 *              variables as members to avoid indirection when manipulating I/O registers
 */

#include "Motors.h"
#include "Hardware.h"

#ifdef DEBUG
#include "Debug.h"
#endif

#include "stepper.h"

#define SOFTWARE_INTERRUPT_PERIOD 99 // Cycles (less one) before interrupt is actually generated after software interrupt is called

/*
 * Initialize motor related I/O and subsystems
 */

void Motors::Initialize(MotorController_t* mcState)
{
    // Initialize stepper motor control system
    st_init(mcState);

    // Setup DDA timer
    TIMER_DDA_CTRLB &= ~TIMER_DDA_CS_BM;
    TIMER_DDA_CTRLB |= (1<<WGM12);
    TIMER_DDA_IMSK |= (1<<OCIE1A); // Generate interrupt on compare

    // Setup load software interrupt timer
    TIMER_LOAD_CTRLB &= ~TIMER_LOAD_CS_BM;
    TIMER_LOAD_CTRLA |= (1<<WGM21); // Clear on compare
    TIMER_LOAD_IMSK |= (1<<OCIE2A); // Generate interrupt on compare
    TIMER_LOAD_PERIOD = SOFTWARE_INTERRUPT_PERIOD;

    // Setup exec software interrupt timer
    TIMER_EXEC_CTRLB &= ~TIMER_EXEC_CS_BM;
    TIMER_EXEC_CTRLA |= (1<<WGM01); // Clear on compare
    TIMER_EXEC_IMSK |= (1<<OCIE0A); // Generate interrupt on compare
    TIMER_EXEC_PERIOD = SOFTWARE_INTERRUPT_PERIOD;

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
    // SLEEP and RESET need to be driven high for device operation
    MOTOR_SLEEP_PORT |= MOTOR_SLEEP_BM;
    MOTOR_RESET_PORT |= MOTOR_RESET_BM;

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

