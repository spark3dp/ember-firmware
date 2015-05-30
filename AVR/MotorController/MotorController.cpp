/*
 * MotorController.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-01
 * Description: Top level motor controller commands
 */

#include <util/delay.h>
#include <string.h>

#include "MotorController.h"
#include "planner.h"
#include "Motors.h"
#include "Hardware.h"
#include "../../C++/include/MotorController.h" // Shared header defining commands

#ifdef DEBUG
#include "Debug.h"
#endif

extern uint32_t stepCount[AXES_COUNT];  // Defined in stepper.c

cmSingleton_t cm;

/*
 * Initialize I/O and subsystems
 */

void MotorController::Initialize(MotorController_t* mcState)
{
    // Set up interrupt signal I/0
    INTERRUPT_DDR |= INTERRUPT_DD_BM;
    // The interrupt is active low so set the pin high to initialize
    INTERRUPT_PORT |= INTERRUPT_BM;

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

    Motors::Initialize(mcState);

    // Initialize planning module
    mp_init();
    
    memset(&cm, 0, sizeof(cm));
}

/*
 * Reinitialize hardware and data structures
 */

void MotorController::Reset(MotorController_t* mcState)
{
    Initialize(mcState);

    // Clear motor controller state, preserving the current state machine state
    MotorController_state_t currentState = mcState->sm_state;
    memset(mcState, 0, sizeof(MotorControllerState));
    mcState->sm_state = currentState;
}

/*
 * Generate a 50ms low pulse on the otherwise high interrupt signal line
 * This function blocks for the pulse duration
 */

void MotorController::GenerateInterrupt()
{
    INTERRUPT_PORT &= ~INTERRUPT_BM;
    _delay_ms(50);
    INTERRUPT_PORT |= INTERRUPT_BM;
}

/*
 * Inspect settings event data and update specified settings object accordingly
 */

void MotorController::UpdateSettings(uint8_t axis, EventData eventData, AxisSettings& axisSettings)
{
    switch(eventData.command)
    {
        case MC_STEP_ANGLE:
            axisSettings.SetStepAngle(eventData.parameter);
            mp_set_pulses_per_unit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_UNITS_PER_REV:
            axisSettings.SetUnitsPerRevolution(eventData.parameter);
            mp_set_pulses_per_unit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_MICROSTEPPING:
            Motors::SetMicrosteppingMode(static_cast<uint8_t>(eventData.parameter));
            axisSettings.SetMicrosteppingMode(static_cast<uint8_t>(eventData.parameter));
            mp_set_pulses_per_unit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_JERK:
            axisSettings.SetMaxJerk(eventData.parameter);
            break;

        case MC_SPEED:
            axisSettings.SetSpeed(eventData.parameter);
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
        Move(Z_AXIS, homingDistance, mcState->zAxisSettings);
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
        Move(R_AXIS, homingDistance, mcState->rAxisSettings);
    }
}

/*
 * Begin decelerating to a pause
 * Setting the hold state to causes the line segment execution to start a hold
 */

void MotorController::BeginMotionHold()
{
    cm.motion_state = MOTION_HOLD;
    cm.hold_state = FEEDHOLD_SYNC;
}

void MotorController::EndMotionHold()
{
    cm.hold_state = FEEDHOLD_END_HOLD;
    mp_end_hold();
}

/*
 * Enqueue a movement block into the planning buffer
 * axisIndex The index corresponding to the axis to move
 * distance The distance to move
 * settings The settings for the axis to move
 */
Status MotorController::Move(uint8_t axisIndex, int32_t distance, const AxisSettings& settings)
{
    Status settingsStatus = settings.Validate();
    if (settingsStatus != MC_STATUS_SUCCESS) return settingsStatus;

    stepCount[Z_AXIS] = 0;
    stepCount[R_AXIS] = 0;
    // TODO: set error if speed, max speed, pulses per unit, or max jerk are zero or if distance is less than some minimum

    // Make the current machine position zero, all moves are relative
    mp_set_axis_position(Z_AXIS, 0.0);
    mp_set_axis_position(R_AXIS, 0.0);

#ifdef DEBUG
    printf_P(PSTR("DEBUG: in MotorController::Move, axis index: %d, distance: %ld, pulses per unit: %f, max jerk: %e\n"),
            axisIndex, distance, static_cast<double>(settings.PulsesPerUnit()), settings.MaxJerk());
#endif
    
    if (cm.cycle_state == CYCLE_OFF)
        cm.cycle_state = CYCLE_MACHINING;

    float distances[AXES_COUNT] = { 0 };
    uint8_t directions[AXES_COUNT];

    distances[axisIndex] = static_cast<float>(distance);
    
    // Handle movement direction with a separate flag
    directions[Z_AXIS] = distances[Z_AXIS] < 0 ? 1 : 0;
    directions[R_AXIS] = distances[R_AXIS] < 0 ? 1 : 0;

    // The motion planning system does not properly deal with negative distances
    distances[axisIndex] = fabs(distances[axisIndex]);
    
    return mp_aline(distances, directions, settings.Speed(), settings.MaxJerk());
}

/*
 * Reset the motion planning buffers and clear the canonical machine internal state
 */

void MotorController::EndMotion()
{
#ifdef DEBUG
    printf_P(PSTR("DEBUG: motion complete, total step pulses generated: Z axis: %ld, R axis: %ld\n"), stepCount[Z_AXIS], stepCount[R_AXIS]);
#endif
    // Clear planning buffer
    // also see mp_flush_planner() in tinyg - planner.c for notes about flushing
    mp_init_buffers();

    if (cm.cycle_state == CYCLE_MACHINING)
    {
      cm.motion_state = MOTION_STOP;
      cm.cycle_state = CYCLE_OFF;
      cm.hold_state = FEEDHOLD_OFF;
      mp_zero_segment_velocity();
    }
}

