/*
 * Motors.cpp
 * Author: Jason Lefley
 * Date  : 2015-04-28
 * Description: Interface to stepper motors
 *              Executes motion plan by dequeuing segments and generating
 *              appropriate step pulses
 *              For more information see stepper.c and stepper.h from TinyG
 *              Attribution: TinyG, Copyright (c) 2010 - 2013 Alden S. Hart Jr.
 */

#include <string.h> // memset
#include <math.h>   // isinfinite
#include <avr/interrupt.h>

#include "tinyg.h"
#include "util.h"

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

/*
 * DDA_SUBSTEPS sets the amount of fractional precision for substepping.
 * Substepping is kind of like microsteps done in software to make
 * interpolation more accurate.
 *
 * Set to 1 to disable, but don't do this or you will lose a lot of accuracy.
 */

#define DDA_SUBSTEPS 100000   // 100,000 accumulates substeps to 6 decimal places

/* You want to reset the DDA accumulators if the new ticks value is way less 
 * than previous value, but otherwise you should leave the accumulators alone.
 * Preserving the accumulator value from the previous segment aligns pulse 
 * phasing between segments. However, if the new accumulator is going to be 
 * much less than the old one you must reset it or risk motor stalls.
 */

#define ACCUMULATOR_RESET_FACTOR 2  // amount counter range can safely change


// Runtime structs. Used exclusively by step generation ISR (HI)
struct stRunMotor // one per controlled motor
{     
  int32_t phase_increment;   // total steps in axis times substeps factor
  int32_t phase_accumulator; // DDA phase angle accumulator for axis
  uint8_t polarity;          // 0=normal polarity, 1=reverse motor polarity
};

struct stRunSingleton // Stepper static values and axis parameters
{   
  int32_t dda_ticks_downcount;  // tick down-counter (unscaled)
  int32_t dda_ticks_X_substeps; // ticks multiplied by scaling factor
  stRunMotor m[AXES_COUNT];     // runtime motor structures
};

// Prep-time structs. Used by exec/prep ISR (MED) and read-only during load 
// Must be careful about volatiles in this one

enum prepBufferState
{
  PREP_BUFFER_OWNED_BY_LOADER = 0, // staging buffer is ready for load
  PREP_BUFFER_OWNED_BY_EXEC        // staging buffer is being loaded
};

struct stPrepMotor
{
  uint32_t phase_increment; // total steps in axis times substep factor
  int8_t dir;               // b0 = direction
};

struct stPrepSingleton
{
  uint8_t move_type;             // move type
  uint8_t prep_state;            // set TRUE to load, false to skip
  volatile uint8_t exec_state;   // move execution state 
  volatile uint8_t reset_flag;   // TRUE if accumulator should be reset
  uint32_t prev_ticks;           // tick count from previous move
  uint32_t dda_ticks;            // DDA or dwell ticks for the move
  uint32_t dda_ticks_X_substeps; // DDA ticks scaled by substep factor
  stPrepMotor m[AXES_COUNT];     // per-motor structs
};

static stRunSingleton st;
static stPrepSingleton sps;
static MotorController_t* mcState;
uint32_t stepCount[AXES_COUNT] = { 0 };

static void _exec_move();
static void _load_move();
static void _request_load_move();

/*
 * DDA timer ISR
 * Generates step pulses to complete move execution
 */

ISR(DDA_TIMER_ISR_vect)
{

  if ((st.m[Z_AXIS].phase_accumulator += st.m[Z_AXIS].phase_increment) > 0) {
    MOTOR_Z_STEP_PORT |= MOTOR_Z_STEP_BM;  // turn step bit on
    st.m[Z_AXIS].phase_accumulator -= st.dda_ticks_X_substeps;
    MOTOR_Z_STEP_PORT &= ~MOTOR_Z_STEP_BM; // turn step bit off in ~1 uSec
    stepCount[Z_AXIS]++;
  }

  if ((st.m[R_AXIS].phase_accumulator += st.m[R_AXIS].phase_increment) > 0) {
    MOTOR_R_STEP_PORT |= MOTOR_R_STEP_BM;  // turn step bit on
    st.m[R_AXIS].phase_accumulator -= st.dda_ticks_X_substeps;
    MOTOR_R_STEP_PORT &= ~MOTOR_R_STEP_BM; // turn step bit off in ~1 uSec
    stepCount[R_AXIS]++;
  }
  
  if (--st.dda_ticks_downcount == 0) {    // end move
      DDA_TIMER_CTRLB &= ~DDA_TIMER_CS_BM; // Disable DDA timer
      _load_move();             // load the next move
  }

}

/*
 * Move execution software interrupt ISR
 * Responds to interrupt
 */

ISR(EXEC_TIMER_ISR_vect)
{
    // Disable timer
    EXEC_TIMER_CTRLB &= ~EXEC_TIMER_CS_BM;
    _exec_move();
}

/*
 * Move execution handler
 */

static void _exec_move()
{
    if (sps.exec_state == PREP_BUFFER_OWNED_BY_EXEC) {
    if (PlannerBufferPool::ExecuteRunBuffer() != STAT_NOOP) {
      sps.exec_state = PREP_BUFFER_OWNED_BY_LOADER; // flip it back
      _request_load_move();
    }
  }
}

/*
 * Request load of next move using timer-driven software interrupt
 */

static void _request_load_move()
{
    // only fire an interrupt if the load routine is ready
  if (st.dda_ticks_downcount == 0)
        LOAD_TIMER_CTRLB |= LOAD_TIMER_CS_BM;
}

/*
 * Load move software interrupt ISR
 * Responds to interrupt
 */

ISR(LOAD_TIMER_ISR_vect)
{
    LOAD_TIMER_CTRLB &= ~LOAD_TIMER_CS_BM; // Disable load software interrupt timer
    _load_move();
}

/*
 * Dequeue move and load into stepper struct
 *
 * This routine can only be called be called from an ISR at the same or 
 * higher level as the DDA or dwell ISR. A software interrupt has been 
 * provided to allow a non-ISR to request a load (see st_request_load_move())
 */

void _load_move()
{
  if (st.dda_ticks_downcount != 0) return;          // exit if it's still busy
  if (sps.exec_state != PREP_BUFFER_OWNED_BY_LOADER)
  {
      // there are no more moves
      mcState->motionComplete = true;
      return;
  }

  // handle aline loads first (most common case)  NB: there are no more lines, only alines
  if (sps.move_type == MOVE_TYPE_ALINE) {
    st.dda_ticks_downcount = sps.dda_ticks;
    st.dda_ticks_X_substeps = sps.dda_ticks_X_substeps;
    //TIMER_DDA_PERIOD = sps.dda_period;
 
    // This section is somewhat optimized for execution speed 
    // All axes must set steps and compensate for out-of-range pulse phasing. 
    // If axis has 0 steps the direction setting can be omitted
    // If axis has 0 steps enabling motors is req'd to support power mode = 1

    st.m[Z_AXIS].phase_increment = sps.m[Z_AXIS].phase_increment;     // set steps
    if (sps.reset_flag == true) {       // compensate for pulse phasing
      st.m[Z_AXIS].phase_accumulator = -(st.dda_ticks_downcount);
    }
    if (st.m[Z_AXIS].phase_increment != 0) {
      // For ideal optimizations, only set or clear a bit at a time.
      if (sps.m[Z_AXIS].dir == 0) {
        MOTOR_Z_DIRECTION_PORT &= ~MOTOR_Z_DIRECTION_BM; // CW motion (bit cleared)
      } else {
        MOTOR_Z_DIRECTION_PORT |= MOTOR_Z_DIRECTION_BM;  // CCW motion
      }
      // should already be enabled MOTOR_ENABLE_PORT &= ~MOTOR_ENABLE_BM;
    }

    st.m[R_AXIS].phase_increment = sps.m[R_AXIS].phase_increment;
    if (sps.reset_flag == true) {
      st.m[R_AXIS].phase_accumulator = -(st.dda_ticks_downcount);
    }
    if (st.m[R_AXIS].phase_increment != 0) {
      if (sps.m[R_AXIS].dir == 0) {
        MOTOR_R_DIRECTION_PORT &= ~MOTOR_R_DIRECTION_BM; // CW motion (bit cleared)
      } else {
        MOTOR_R_DIRECTION_PORT |= MOTOR_R_DIRECTION_BM;  // CCW motion
      }
    }

    // Enable the DDA timer
    // For some reason, the timer will not start properly unless the count and interrupt flag are cleared
    // According to the data sheet this occurs when the ISR executes
    DDA_TIMER_IFR |= DDA_TIMER_OCF_BM;
    DDA_TIMER_CNT = 0;
    DDA_TIMER_CTRLB |= DDA_TIMER_CS_BM;
  }

  // all other cases drop to here (e.g. Null moves after Mcodes skip to here) 
  sps.exec_state = PREP_BUFFER_OWNED_BY_EXEC;       // flip it back
  sps.prep_state = false;
  Motors::RequestMoveExecution();                 // exec and prep next move
}

/*
 * Initialize motor related I/O and subsystems
 */

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
    memset(&st, 0, sizeof(st));
    memset(&sps, 0, sizeof(sps));
    sps.exec_state = PREP_BUFFER_OWNED_BY_EXEC;
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
 * Prepare the next move for the loader
 *
 * This function does the math on the next pulse segment and gets it ready for 
 * the loader. It deals with all the DDA optimizations and timer setups so that
 * loading can be performed as rapidly as possible. It works in joint space 
 * (motors) and it works in steps, not length units. All args are provided as 
 * floats and converted to their appropriate integer types for the loader. 
 *
 * steps Relative motion in steps (can be non-integer values)
 * directions Flags to specify motor rotation direction
 * microseconds How many microseconds the segment should run for
 */

Status Motors::SetNextSegment(float steps[], uint8_t directions[], float microseconds)
{
    float f_dda = F_DDA;    // starting point for adjustment
    float dda_substeps = DDA_SUBSTEPS;

    // trap conditions that would prevent queueing the line
    if (sps.exec_state != PREP_BUFFER_OWNED_BY_EXEC)
        return (STAT_INTERNAL_ERROR);
    else if (isfinite(microseconds) == false)
        return (STAT_MINIMUM_LENGTH_MOVE_ERROR);
    else if (microseconds < EPSILON)
        return (STAT_MINIMUM_TIME_MOVE_ERROR);

    sps.reset_flag = false;   // initialize accumulator reset flag for this move.

    // setup motor parameters
    sps.m[Z_AXIS].dir = directions[Z_AXIS] ^ Z_AXIS_MOTOR_POLARITY;
    sps.m[Z_AXIS].phase_increment = (uint32_t)fabs(steps[Z_AXIS] * dda_substeps);
    sps.m[R_AXIS].dir = directions[R_AXIS] ^ R_AXIS_MOTOR_POLARITY;
    sps.m[R_AXIS].phase_increment = (uint32_t)fabs(steps[R_AXIS] * dda_substeps);
    sps.dda_ticks = (uint32_t)((microseconds/1000000) * f_dda);
    sps.dda_ticks_X_substeps = sps.dda_ticks * dda_substeps;  // see note about substeps

    // anti-stall measure in case change in velocity between segments is too great 
    if ((sps.dda_ticks * ACCUMULATOR_RESET_FACTOR) < sps.prev_ticks) // NB: uint32_t math
        sps.reset_flag = true;

    sps.prev_ticks = sps.dda_ticks;
    sps.move_type = MOVE_TYPE_ALINE;
    sps.prep_state = true;
    return (STAT_OK);
}

/*
 * Performs no action other than setting flags to keep loader
 * in a consistent state
 */

void Motors::SetNextSegmentNull()
{
    sps.move_type = MOVE_TYPE_NULL;
    sps.prep_state = true;
}

/*
 * Request execution of next move using timer-driven software interrupt
 */

void Motors::RequestMoveExecution()
{
    if (sps.exec_state == PREP_BUFFER_OWNED_BY_EXEC)
        EXEC_TIMER_CTRLB |= EXEC_TIMER_CS_BM;
}
