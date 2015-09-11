//  File: Motors.cpp
//  Interface to stepper motors
//  Executes motion plan by dequeuing segments and generating appropriate step pulses
//  For more information see stepper.c and stepper.h from TinyG
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from TinyG <https://www.synthetos.com/project/tinyg/>.
//
//  Copyright 2010 - 2015 Alden S. Hart Jr.
//  Copyright 2013 - 2015 Robert Giseburt
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

#include <string.h>
#include <math.h>
#include <avr/interrupt.h>

#include "Utils.h"
#include "Motors.h"
#include "Hardware.h"
#include "MachineDefinitions.h"
#include "PlannerBufferPool.h"

#ifdef DEBUG
#include "Debug.h"
#endif

#define _f_to_period(f) (uint16_t)((float)F_CPU / (float)f)
#define F_DDA (float)40000  // DDA frequency in hz.
#define SOFTWARE_INTERRUPT_PERIOD 99 // Cycles (less one) before interrupt is actually generated after software interrupt is called

// DDA_SUBSTEPS sets the amount of fractional precision for substepping.
// Substepping is kind of like microsteps done in software to make
// interpolation more accurate.
// Set to 1 to disable, but don't do this or you will lose a lot of accuracy.
#define DDA_SUBSTEPS 100000   // 100,000 accumulates substeps to 6 decimal places

// You want to reset the DDA accumulators if the new ticks value is way less 
// than previous value, but otherwise you should leave the accumulators alone.
// Preserving the accumulator value from the previous segment aligns pulse 
// phasing between segments. However, if the new accumulator is going to be 
// much less than the old one you must reset it or risk motor stalls.
#define ACCUMULATOR_RESET_FACTOR 2  // amount counter range can safely change


// Runtime structures - used exclusively by step generation ISR (HI)
struct MotorRuntimeState // one per controlled motor
{     
    int32_t phaseIncrement;   // total steps in axis times substeps factor
    int32_t phaseAccumulator; // DDA phase angle accumulator for axis
};

struct RuntimeState // Stepper static values and axis parameters
{   
    int32_t ddaTicksDowncount;                  // tick down-counter (unscaled)
    int32_t ddaTicksXSubsteps;                  // ticks multiplied by scaling factor
    MotorRuntimeState motorRuntime[AXES_COUNT]; // runtime motor structures
};

// Prep-time structures - used by exec/prep ISR (MED) and read-only during load 
// Must be careful about volatiles in this one
enum PrepBufferState
{
    PREP_BUFFER_OWNED_BY_LOADER = 0, // staging buffer is ready for load
    PREP_BUFFER_OWNED_BY_EXEC        // staging buffer is being loaded
};

struct MotorPrepState
{
    uint32_t phaseIncrement; // total steps in axis times substep factor
    int8_t direction;        // direction flag
};

struct PrepState
{
    uint8_t moveType;                     // move type
    uint8_t prepState;                    // set TRUE to load, false to skip
    volatile PrepBufferState executionState; // move execution state 
    volatile uint8_t resetFlag;           // TRUE if accumulator should be reset
    uint32_t previousTicks;               // tick count from previous move
    uint32_t ddaTicks;                    // DDA or dwell ticks for the move
    uint32_t ddaTicksXSubsteps;           // DDA ticks scaled by substep factor
    MotorPrepState motorPrep[AXES_COUNT]; // per-motor structs
};

static RuntimeState runtimeState;
static PrepState prepState;
static MotorController_t* mcState;

#ifdef DEBUG
uint32_t stepCount[AXES_COUNT] = { 0 };
#endif

static void loadMove();

// Initialize motor related I/O and subsystems
void Motors::Initialize(MotorController_t* mc)
{
    mcState = mc;

    // Setup DDA timer
    // Clear timer and generate interrupt on compare match
    DDA_TIMER_CTRLA  = 0;
    DDA_TIMER_CTRLB  = DDA_TIMER_WGM_BM;
    DDA_TIMER_IMSK   = DDA_TIMER_OCIE_BM;
    DDA_TIMER_PERIOD = _f_to_period(F_DDA);

    // Setup load software interrupt timer
    // Clear timer and generate interrupt on compare match
    LOAD_TIMER_CTRLA  = LOAD_TIMER_WGM_BM;
    LOAD_TIMER_CTRLB  = 0;
    LOAD_TIMER_IMSK   = LOAD_TIMER_OCIE_BM;
    LOAD_TIMER_PERIOD = SOFTWARE_INTERRUPT_PERIOD;

    // Setup exec software interrupt timer
    // Clear timer and generate interrupt on compare match
    EXEC_TIMER_CTRLA  = EXEC_TIMER_WGM_BM;
    EXEC_TIMER_CTRLB  = 0;
    EXEC_TIMER_IMSK   = EXEC_TIMER_OCIE_BM;
    EXEC_TIMER_PERIOD =  SOFTWARE_INTERRUPT_PERIOD;

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

    // From DRV8825 datasheet:
    // When the reset pin is driven low, the internal logic is reset and
    // the step table is reset to the home position
    // RESET needs to be driven high for device operation

    MOTOR_RESET_PORT &= ~MOTOR_RESET_BM;
    MOTOR_RESET_PORT |= MOTOR_RESET_BM;

    // Disable drivers
    Disable();

    // Initialize struct instances
    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&prepState, 0, sizeof(prepState));
    prepState.executionState = PREP_BUFFER_OWNED_BY_EXEC;
}

// Set the microstepping mode
// Mode signals from each chip are tied together
// modeFlag A flag determining the microstepping mode to use
//          1 = full step, 2 = half step, ... 6 = 1/32 step
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
            // This function expects external error checking to catch invalid modes
            break;
    }
}

// Enable the motor drivers
// Both chips are tied to the same enable signal
void Motors::Enable()
{
    // From DRV8825 datasheet:
    // When ENABLE is driven low, device is enabled
    MOTOR_ENABLE_PORT &= ~MOTOR_ENABLE_BM;
}

// Disable the motor drivers
// Both chips are tied to the same enable signal
void Motors::Disable()
{
    MOTOR_ENABLE_PORT |= MOTOR_ENABLE_BM;
}

// Prepare the next move for the loader
// This function does the math on the next pulse segment and gets it ready for 
// the loader. It deals with all the DDA optimizations and timer setups so that
// loading can be performed as rapidly as possible. It works in joint space 
// (motors) and it works in steps, not length units. All args are provided as 
// floats and converted to their appropriate integer types for the loader. 
// steps Relative motion in steps (can be non-integer values)
// directions Flags to specify motor rotation direction
// microseconds How many microseconds the segment should run for
Status Motors::SetNextSegment(float steps[], uint8_t directions[], float microseconds)
{
    float fDDA = F_DDA; // Starting point for adjustment
    float ddaSubsteps = DDA_SUBSTEPS;

    // Trap conditions that would prevent queueing the line
    if (prepState.executionState != PREP_BUFFER_OWNED_BY_EXEC)
        return MC_STATUS_INTERNAL_ERROR;
    else if (isfinite(microseconds) == false)
        return MC_STATUS_MOVE_LENGTH_TOO_SMALL;
    else if (microseconds < EPSILON)
        return MC_STATUS_MOVE_TIME_TOO_SMALL;

    prepState.resetFlag = false; // initialize accumulator reset flag for this move.

    // Setup motor parameters
    prepState.motorPrep[Z_AXIS].direction = directions[Z_AXIS] ^ Z_AXIS_MOTOR_POLARITY;
    prepState.motorPrep[Z_AXIS].phaseIncrement = (uint32_t)fabs(steps[Z_AXIS] * ddaSubsteps);

    prepState.motorPrep[R_AXIS].direction = directions[R_AXIS] ^ R_AXIS_MOTOR_POLARITY;
    prepState.motorPrep[R_AXIS].phaseIncrement = (uint32_t)fabs(steps[R_AXIS] * ddaSubsteps);

    prepState.ddaTicks = (uint32_t)((microseconds/1000000) * fDDA);
    prepState.ddaTicksXSubsteps = prepState.ddaTicks * ddaSubsteps;  // see note about substeps

    // anti-stall measure in case change in velocity between segments is too great 
    if ((prepState.ddaTicks * ACCUMULATOR_RESET_FACTOR) < prepState.previousTicks) // NB: uint32_t math
        prepState.resetFlag = true;

    prepState.previousTicks = prepState.ddaTicks;
    prepState.moveType = MOVE_TYPE_ALINE;
    prepState.prepState = true;
    return MC_STATUS_SUCCESS;
}

// Performs no action other than setting flags to keep loader
// in a consistent state
void Motors::SetNextSegmentNull()
{
    prepState.moveType = MOVE_TYPE_NULL;
    prepState.prepState = true;
}

// Request execution of next move using timer-driven software interrupt
void Motors::RequestMoveExecution()
{
    if (prepState.executionState == PREP_BUFFER_OWNED_BY_EXEC)
        EXEC_TIMER_CTRLB |= EXEC_TIMER_CS_BM;
}

// Dequeue move and load into stepper struct
// This routine can only be called be called from an ISR at the same or 
// higher level as the DDA timer ISR (DDA or load)
static void loadMove()
{
    if (runtimeState.ddaTicksDowncount != 0) return; // Exit if it's still busy

    if (prepState.executionState != PREP_BUFFER_OWNED_BY_LOADER)
    {
        // There are no more moves
        mcState->motionComplete = true;
        return;
    }

    // handle aline loads first (most common case)
    if (prepState.moveType == MOVE_TYPE_ALINE)
    {
        runtimeState.ddaTicksDowncount = prepState.ddaTicks;
        runtimeState.ddaTicksXSubsteps = prepState.ddaTicksXSubsteps;

        // This section is somewhat optimized for execution speed 
        // All axes must set steps and compensate for out-of-range pulse phasing
        // If axis has 0 steps the direction setting can be omitted

        // Set steps
        runtimeState.motorRuntime[Z_AXIS].phaseIncrement = prepState.motorPrep[Z_AXIS].phaseIncrement;

        if (prepState.resetFlag == true)
            // Compensate for pulse phasing
            runtimeState.motorRuntime[Z_AXIS].phaseAccumulator = -(runtimeState.ddaTicksDowncount);
        
        if (runtimeState.motorRuntime[Z_AXIS].phaseIncrement != 0)
        {
            // For ideal optimizations, only set or clear a bit at a time.
            if (prepState.motorPrep[Z_AXIS].direction == 0)
                MOTOR_Z_DIRECTION_PORT &= ~MOTOR_Z_DIRECTION_BM; // CW motion (bit cleared)
            else
                MOTOR_Z_DIRECTION_PORT |= MOTOR_Z_DIRECTION_BM;  // CCW motion
        }

        // Set steps
        runtimeState.motorRuntime[R_AXIS].phaseIncrement = prepState.motorPrep[R_AXIS].phaseIncrement;

        if (prepState.resetFlag == true)
            // Compensate for pulse phasing
            runtimeState.motorRuntime[R_AXIS].phaseAccumulator = -(runtimeState.ddaTicksDowncount);
        
        if (runtimeState.motorRuntime[R_AXIS].phaseIncrement != 0)
        {
            // For ideal optimizations, only set or clear a bit at a time.
            if (prepState.motorPrep[R_AXIS].direction == 0)
                MOTOR_R_DIRECTION_PORT &= ~MOTOR_R_DIRECTION_BM; // CW motion (bit cleared)
            else
                MOTOR_R_DIRECTION_PORT |= MOTOR_R_DIRECTION_BM;  // CCW motion
        }

        // Enable the DDA timer
        // For some reason, the timer will not start properly unless the count and interrupt flag are cleared
        // According to the data sheet this occurs when the ISR executes
        DDA_TIMER_IFR |= DDA_TIMER_OCF_BM;
        DDA_TIMER_CNT = 0;
        DDA_TIMER_CTRLB |= DDA_TIMER_CS_BM;
    }

    // All other cases drop to here (e.g. null moves) 
    // Set execution state to indicate completion of loading
    prepState.executionState = PREP_BUFFER_OWNED_BY_EXEC;
    prepState.prepState = false;

    // Execute and prepare next move
    Motors::RequestMoveExecution();
}

// DDA timer ISR
// Generates step pulses to complete move execution
// Step pulses are generated by transitioning the step pin from low to high and high to low
ISR(DDA_TIMER_ISR_vect)
{

    if ((runtimeState.motorRuntime[Z_AXIS].phaseAccumulator += runtimeState.motorRuntime[Z_AXIS].phaseIncrement) > 0)
    {
        MOTOR_Z_STEP_PORT |= MOTOR_Z_STEP_BM;
        runtimeState.motorRuntime[Z_AXIS].phaseAccumulator -= runtimeState.ddaTicksXSubsteps;
        MOTOR_Z_STEP_PORT &= ~MOTOR_Z_STEP_BM;
#ifdef DEBUG
        stepCount[Z_AXIS]++;
#endif
    }

    if ((runtimeState.motorRuntime[R_AXIS].phaseAccumulator += runtimeState.motorRuntime[R_AXIS].phaseIncrement) > 0)
    {
        MOTOR_R_STEP_PORT |= MOTOR_R_STEP_BM;
        runtimeState.motorRuntime[R_AXIS].phaseAccumulator -= runtimeState.ddaTicksXSubsteps;
        MOTOR_R_STEP_PORT &= ~MOTOR_R_STEP_BM;
#ifdef DEBUG
        stepCount[R_AXIS]++;
#endif
    }

    if (--runtimeState.ddaTicksDowncount == 0)
    {
        // The move corresponding to the currently loaded segment completed
        // Disable the DDA timer and load the next segment
        DDA_TIMER_CTRLB &= ~DDA_TIMER_CS_BM;
        loadMove();
    }
}

// Move execution software interrupt ISR
// Responds to interrupt
ISR(EXEC_TIMER_ISR_vect)
{
    // Disable timer
    EXEC_TIMER_CTRLB &= ~EXEC_TIMER_CS_BM;
  
    // Verify state
    if (prepState.executionState == PREP_BUFFER_OWNED_BY_EXEC)
    {
        Status status = PlannerBufferPool::ExecuteRunBuffer();

        if (status == MC_STATUS_INTERNAL_ERROR)
        {
            // Raise error if internal error occurs
            mcState->status = status;
            mcState->error = true;
            
        }
        else if (status != MC_STATUS_NOOP)
        {
            // Execution complete
            prepState.executionState = PREP_BUFFER_OWNED_BY_LOADER;
            
            // Only fire an interrupt to load the next move if the currently loaded move is complete
            if (runtimeState.ddaTicksDowncount == 0)
                LOAD_TIMER_CTRLB |= LOAD_TIMER_CS_BM;
        }
    }
}

// Load move software interrupt ISR
// Responds to interrupt
ISR(LOAD_TIMER_ISR_vect)
{
    LOAD_TIMER_CTRLB &= ~LOAD_TIMER_CS_BM; // Disable load software interrupt timer
    loadMove();
}

