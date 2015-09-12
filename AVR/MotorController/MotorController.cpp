//  File: MotorController.cpp
//  Top level motor controller commands
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <util/delay.h>
#include <string.h>

#include "MotorController.h"
#include "Planner.h"
#include "Motors.h"
#include "Hardware.h"
#include "MachineDefinitions.h"
#include "PlannerBufferPool.h"
#include "../../C++/include/MotorController.h" // Shared header defining commands

#ifdef DEBUG
#include "Debug.h"
#endif

extern uint32_t stepCount[AXES_COUNT];  // Defined in Motors.cpp

// Initialize I/O and subsystems
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
    Planner::Initialize(mcState);
    PlannerBufferPool::Initialize();
}

// Generate a 50ms low pulse on the otherwise high interrupt signal line
// This function blocks for the pulse duration
void MotorController::GenerateInterrupt()
{
    INTERRUPT_PORT &= ~INTERRUPT_BM;
    _delay_ms(50);
    INTERRUPT_PORT |= INTERRUPT_BM;
}

// Inspect settings event data and update specified settings object accordingly
Status MotorController::UpdateSettings(uint8_t axis, EventData eventData, AxisSettings& axisSettings)
{
    switch(eventData.command)
    {
        case MC_STEP_ANGLE:
            RETURN_ON_ERROR(axisSettings.SetStepAngle(eventData.parameter));
            Planner::SetPulsesPerUnit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_UNITS_PER_REV:
            RETURN_ON_ERROR(axisSettings.SetUnitsPerRevolution(eventData.parameter));
            Planner::SetPulsesPerUnit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_MICROSTEPPING:
            RETURN_ON_ERROR(axisSettings.SetMicrosteppingMode(static_cast<uint8_t>(eventData.parameter)));
            Motors::SetMicrosteppingMode(static_cast<uint8_t>(eventData.parameter));
            Planner::SetPulsesPerUnit(axis, axisSettings.PulsesPerUnit());
            break;

        case MC_JERK:
            RETURN_ON_ERROR(axisSettings.SetMaxJerk(eventData.parameter));
            break;

        case MC_SPEED:
            RETURN_ON_ERROR(axisSettings.SetSpeed(eventData.parameter));
            break;

        default:
            return MC_STATUS_SETTING_COMMAND_UNKNOWN;
            break;
    }

    return MC_STATUS_SUCCESS;
}

// Home the z axis
// If the axis is already home, raise the appropriate event
// Otherwise enable ping change interrupt for z axis limit switch and begin homing movement
// homingDistance The distance in units to move when queueing movement
// mcState The global state struct instance, used to raise state machine event
Status MotorController::HomeZAxis(int32_t homingDistance, MotorController_t* mcState)
{
    if (Z_AXIS_LIMIT_SW_HIT)
    {
        // Already at home, set flag to trigger exit from homing state
        mcState->axisAtLimit = true;
        return MC_STATUS_SUCCESS;
    }
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeZAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= Z_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        return Move(Z_AXIS, homingDistance, mcState->zAxisSettings);
    }
}

// Home the r axis
// If the axis is already home, raise the appropriate event
// Otherwise enable ping change interrupt for r axis limit switch and begin homing movement
// homingDistance The distance in units to move when queueing movement
// mcState The global state instance, used to raise state machine event
Status MotorController::HomeRAxis(int32_t homingDistance, MotorController_t* mcState)
{
    if (R_AXIS_LIMIT_SW_HIT)
    {
        // Already at home, set flag to trigger exit from homing state
        mcState->axisAtLimit = true;
        return MC_STATUS_SUCCESS;
    }
    else
    {
#ifdef DEBUG
        printf_P(PSTR("DEBUG: in MotorController::HomeRAxis, axis not at home, enabling interrupt and beginning motion\n"));
#endif
        // Enable pin change interrupt
        LIMIT_SW_PCMSK |= R_AXIS_LIMIT_SW_PCINT_BM;
        // Begin homing movement
        return Move(R_AXIS, homingDistance, mcState->rAxisSettings);
    }
}

// Enqueue a movement block into the planning buffer
// axisIndex The index corresponding to the axis to move
// distance The distance to move
// settings The settings for the axis to move
Status MotorController::Move(uint8_t axisIndex, int32_t distance, const AxisSettings& settings)
{
    RETURN_ON_ERROR(settings.Validate());

#ifdef DEBUG
    stepCount[Z_AXIS] = 0;
    stepCount[R_AXIS] = 0;
#endif

    // Make the current machine position zero, all moves are relative
    Planner::SetAxisPosition(Z_AXIS, 0.0);
    Planner::SetAxisPosition(R_AXIS, 0.0);

#ifdef DEBUG
    printf_P(PSTR("DEBUG: in MotorController::Move, axis index: %d, distance: %ld, pulses per unit: %f, max jerk: %e\n"),
            axisIndex, distance, static_cast<double>(settings.PulsesPerUnit()), settings.MaxJerk());
#endif
    
    float distances[AXES_COUNT] = { 0 };
    uint8_t directions[AXES_COUNT];

    distances[axisIndex] = static_cast<float>(distance);
    
    // Handle movement direction with a separate flag
    directions[Z_AXIS] = distances[Z_AXIS] < 0 ? 1 : 0;
    directions[R_AXIS] = distances[R_AXIS] < 0 ? 1 : 0;

    // The motion planning system does not properly deal with negative distances
    distances[axisIndex] = fabs(distances[axisIndex]);
    
    return Planner::PlanAccelerationLine(distances, directions, settings.Speed(), settings.MaxJerk());
}

// Reset the motion planning buffers and clear the canonical machine internal state
void MotorController::EndMotion()
{
#ifdef DEBUG
    printf_P(PSTR("DEBUG: motion complete, total step pulses generated: Z axis: %ld, R axis: %ld\n"), stepCount[Z_AXIS], stepCount[R_AXIS]);
#endif
    // Clear planning buffer and communicate move completion to planner
    PlannerBufferPool::Initialize();
    Planner::EndMove();
}

