/*
 * MotorController.cpp
 * Author: Jason Lefley
 * Date  : 2015-05-01
 * Description: Top level motor controller commands
 */


#include "MotorController.h"
#include "Motors.h"
#include "Hardware.h"
#include "../../C++/include/MotorController.h" // Shared header defining commands

#ifdef DEBUG
#include "Debug.h"
#endif

/*
 * Initialize I/O and subsystems
 */

void MotorController::Initialize()
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

    Motors::Initialize();
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
        // Already at home, raise limit reached event
        MotorController_State_Machine_Event(mcState, &command, AxisLimitReached);
    }
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeZAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= Z_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        Motors::Move(Z_AXIS_MOTOR, homingDistance, mcState->zAxisSettings);
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
        // Already at home, raise limit reached event
        MotorController_State_Machine_Event(mcState, &command, AxisLimitReached);
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeRAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= R_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        Motors::Move(R_AXIS_MOTOR, homingDistance, mcState->rAxisSettings);
    }
}
