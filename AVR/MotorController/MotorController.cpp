/*
 * MotorController.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-01
 * Description: Top level motor controller commands
 */

#include "MotorController.h"
#include "canonical_machine.h"
#include "planner.h"
#include "kinematics.h"
#include "Motors.h"
#include "Hardware.h"
#include "../../C++/include/MotorController.h" // Shared header defining commands

#ifdef DEBUG
#include "Debug.h"
#endif

/*
 * Initialize I/O and subsystems
 */

void MotorController::Initialize(MotorController_t* mcState)
{
    // Set up limit switch I/O
    Z_AXIS_LIMIT_SW_DDR &= ~Z_AXIS_LIMIT_SW_DD_BM;
    R_AXIS_LIMIT_SW_DDR &= ~R_AXIS_LIMIT_SW_DD_BM;
    
    // Enable internal pullups for limit switch pins
    Z_AXIS_LIMIT_SW_PORT |= Z_AXIS_LIMIT_SW_BM;
    R_AXIS_LIMIT_SW_PORT |= R_AXIS_LIMIT_SW_BM;

    // Initialize limit switch pin change interrupt
    PCICR |= LIMIT_SW_PCIE_BM;

    // Ensure pin change interrupts are disabled
    LIMIT_SW_PCMSK &= ~Z_AXIS_LIMIT_SW_PCINT_BM;
    LIMIT_SW_PCMSK &= ~R_AXIS_LIMIT_SW_PCINT_BM;

    /*
     * Subsystems
     */
    
    Motors::Initialize(mcState);

    // Initialize planning buffers
    mp_init();

    // Initialize canonical machine
    cm_init();
}

/*
 * Reset drivers, reinitialize data structures, clear error
 */

void MotorController::Reset()
{
    Motors::Reset();
}

void MotorController::HandleSettingsCommand(Command* command, AxisSettings& axisSettings)
{
    switch(command->Action())
    {
        case MC_STEP_ANGLE:
            axisSettings.SetStepAngle(command->Parameter());
            break;

        case MC_UNITS_PER_REV:
            axisSettings.SetUnitsPerRevolution(command->Parameter());
            break;

        case MC_MICROSTEPPING:
            Motors::SetMicrosteppingMode(static_cast<uint8_t>(command->Parameter()));
            axisSettings.SetMicrosteppingMode(static_cast<uint8_t>(command->Parameter()));
            break;

        case MC_JERK:
            axisSettings.SetMaxJerk(command->Parameter());
            break;

        case MC_SPEED:
            axisSettings.SetSpeed(command->Parameter());
            break;

        case MC_MAX_SPEED:
            axisSettings.SetMaxSpeed(command->Parameter());
            break;

        default:
            //TODO: set error
            break;
    }
}

/*
 * Home the z axis
 * If the axis is already home, raise the appropriate event
 * Otherwise enable ping change interrupt for z axis limit switch and begin homing movement
 * homingDistance The distance in units to move when queueing movement
 * mcState The global state struct instance, used to raise state machine event
 */

void MotorController::HomeZAxis(int32_t homingDistance, MotorController_t* mcState)
{
    Command command;

    if (Z_AXIS_LIMIT_SW_HIT)
    {
        // Already at home, set motion complete flag
        mcState->motionComplete = true;
    }
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeZAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= Z_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        Move(Z_AXIS_MOTOR, homingDistance, mcState->zAxisSettings);
    }
}

/*
 * Home the r axis
 * If the axis is already home, raise the appropriate event
 * Otherwise enable ping change interrupt for r axis limit switch and begin homing movement
 * homingDistance The distance in units to move when queueing movement
 * mcState The global state instance, used to raise state machine event
 */

void MotorController::HomeRAxis(int32_t homingDistance, MotorController_t* mcState)
{
    Command command;

    if (R_AXIS_LIMIT_SW_HIT)
        // Already at home, set motion complete flag
        mcState->motionComplete = true;
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeRAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= R_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        Move(R_AXIS_MOTOR, homingDistance, mcState->rAxisSettings);
    }
}

void MotorController::HandleAxisLimitReached()
{
    cm_begin_feedhold();
}

/*
 * Enqueue a movement block into the planning buffer
 * motorIndex The index corresponding to the motor to move
 * distance The distance to move
 * settings The settings for the axis to move
 */
void MotorController::Move(uint8_t motorIndex, int32_t distance, const AxisSettings& settings)
{
    PulsesPerUnit = settings.PulsesPerUnit();
    MaxJerk = settings.MaxJerk();
#ifdef DEBUG
    printf_P(PSTR("DEBUG: in Motors::Move, motor index: %d, distance: %ld, pulses per unit: %f, max jerk: %e\n"),
            motorIndex, distance, static_cast<double>(PulsesPerUnit), MaxJerk);
#endif
    cm_straight_feed(static_cast<float>(distance), settings.Speed(), settings.MaxSpeed());
}

/*
 * Reset the motion planning buffers and clear the canonical machine internal state
 */

void MotorController::EndMotion()
{
    // Clear planning buffer
    // also see mp_flush_planner() in tinyg - planner.c for notes about flushing
    mp_init_buffers();
    cm_cycle_end();
}
