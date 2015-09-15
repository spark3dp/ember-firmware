//  File: Planner.cpp
//  Motion planning functionality
//  For more information see plan_line.c and planner.h from TinyG
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from TinyG <https://www.synthetos.com/project/tinyg/>.
//
//  Copyright 2010 - 2015 Alden S. Hart Jr.
//  Copyright 2012 - 2015 Rob Giseburt
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

#include "Planner.h"
#include "MachineDefinitions.h"
#include "PlannerBufferPool.h"
#include "Motors.h"
#include "Utils.h"

#define JERK_MATCH_PRECISION 1000 // precision to which jerk must match to be considered effectively the same
#define NOM_SEGMENT_USEC ((float)5000) // nominal segment time
#define MIN_SEGMENT_USEC ((float)2500) // minimum segment time
#define MIN_SEGMENT_TIME (MIN_SEGMENT_USEC / MICROSECONDS_PER_MINUTE)
#define MOVE_STATE_RUN1 MOVE_STATE_RUN // a convenience


// Some parameters for calculateTrapezoid()
// TRAPEZOID_ITERATION_ERROR_PERCENT    Error percentage for iteration convergence. As percent - 0.01 = 1%
// TRAPEZOID_LENGTH_FIT_TOLERANCE       Tolerance for "exact fit" for H and T cases
// TRAPEZOID_VELOCITY_TOLERANCE         Adaptive velocity tolerance term
#define TRAPEZOID_ITERATION_ERROR_PERCENT 0.10
#define TRAPEZOID_LENGTH_FIT_TOLERANCE (0.0001) // allowable mm of error in planning phase
#define TRAPEZOID_VELOCITY_TOLERANCE (max(2, bf->entryVelocity / 100))


// planner routines / hold planning
static float getTargetLength(float Vi, float Vt, const Buffer* bf);
static float getTargetVelocity(float Vi, float L, const Buffer* bf);
static void planBlockList(Buffer* bf, uint8_t* runtimeFlag);
static void resetReplannableList();
static void calculateTrapezoid(Buffer* bf);


// execute routines (NB: These are all called from the LO interrupt)
static Status executeALine(Buffer* bf);
static Status executeALineHead();
static Status executeALineBody();
static Status executeALineTail();
static Status executeALineSegment(uint8_t correction_flag);
static float computeNextSegmentVelocity();
static void initForwardDiffs(float t0, float t2);


// Hold and motion state enumerations
enum HoldState
{
    HOLD_OFF = 0, // no hold in effect
    HOLD_SYNC,    // start hold - sync to latest aline segment
    HOLD_PLAN,    // replan blocks for hold
    HOLD_DECEL,   // decelerate to hold point
    HOLD_HOLD     // holding
};

enum MotionState
{
    MOTION_STOP = 0,  // motion has stopped
    MOTION_RUN,       // machine is in motion
    MOTION_HOLD       // hold in progress
};


// Planner runtime state
// Holds parameters for currently running buffer
struct PlannerRuntimeState
{
    MoveState moveState;              // state of the overall move
    MoveState sectionState;           // state within a move section

    float endpoint[AXES_COUNT];       // final target for Buffer (used to correct rounding errors)
    float position[AXES_COUNT];       // current move position
    float target[AXES_COUNT];         // target move position
    float unit[AXES_COUNT];           // unit vector for axis scaling & planning
    uint8_t direction[AXES_COUNT];    // vector containing flags to indicate movement direction
    float pulsesPerUnit[AXES_COUNT];  // step pulses per unit

    float headLength;                 // copies of Buffer variables of same name
    float bodyLength;
    float tailLength;
    float entryVelocity;
    float cruiseVelocity;
    float exitVelocity;

    float moveTime;                   // total running time (derived)
    float midpointVelocity;           // velocity at accel/decel midpoint
    float jerk;                       // max linear jerk

    float segments;                   // number of segments in arc or blend
    uint32_t segmentCount;            // count of running segments
    float segmentMoveTime;            // actual time increment per aline segment
    float microseconds;               // line or segment time in microseconds
    float segmentVelocity;            // computed velocity for aline segment
    float forwardDiff1;               // forward difference level 1 (Acceleration)
    float forwardDiff2;               // forward difference level 2 (Jerk - constant)
    float previousJerk;               // jerk values cached from previous move
    float previousReciprocalJerk;
    float previousCubeRootJerk;
    MotionState motionState;
    HoldState holdState;
};


static PlannerRuntimeState state;
static MotorController_t* mcState;

// Initialize data structures
void Planner::Initialize(MotorController_t* mc)
{
    mcState = mc;
    memset(&state, 0, sizeof(state)); // clear all values, pointers and status
}

// Set current position of specified axis
void Planner::SetAxisPosition(uint8_t axis, float position)
{
    state.position[axis] = position;
}

// Set step pulses required to move one unit for specified axis
void Planner::SetPulsesPerUnit(uint8_t axis, float value)
{
    state.pulsesPerUnit[axis] = value;
}

// Plan a line with acceleration / deceleration
//
// This function uses constant jerk motion equations to plan acceleration 
// and deceleration. The jerk is the rate of change of acceleration; it's
// the 1st derivative of acceleration, and the 3rd derivative of position. 
// Jerk is a measure of impact to the machine. Controlling jerk smoothes 
// transitions between moves and allows for faster feeds while controlling 
// machine oscillations and other undesirable side-effects.
//
// A detailed explanation of how this module works can be found on the wiki:
// http://www.synthetos.com/wiki/index.php?title=Projects:TinyG-Developer-Info:#Acceleration_Planning
//
// Note: All math is done in absolute coordinates using "float precision" 
// floating point (even though avr-gcc does this as single precision)
//
// Note: Returning a status other than success means the endpoint is NOT
// advanced. So lines that are too short to move will accumulate and get 
// executed once the accumulated error exceeds the minimums 
Status Planner::PlanAccelerationLine(const float distances[], const uint8_t directions[], float speed, float maxJerk)
{
    Buffer* bf;

    // get a cleared buffer and setup move variables
    if ((bf = PlannerBufferPool::GetWriteBuffer()) == NULL) return MC_STATUS_PLANNER_BUFFER_FULL; // never supposed to fail

    // register the callback to the exec function
    bf->executionFunction = executeALine;

    // copy parameters into Buffer
    bf->jerk = maxJerk;
    bf->cruiseVMax = speed;
    CopyAxisVector(bf->direction, directions);

    if (fp_NOT_ZERO(distances[Z_AXIS]))
    {
        bf->length = distances[Z_AXIS];
        bf->unit[Z_AXIS] = 1.0;
        bf->target[Z_AXIS] = distances[Z_AXIS];
    }
    else if (fp_NOT_ZERO(distances[R_AXIS]))
    {
        bf->length = distances[R_AXIS];
        bf->unit[R_AXIS] = 1.0;
        bf->target[R_AXIS] = distances[R_AXIS];
    }
    else
        return MC_STATUS_MOVE_LENGTH_TOO_SMALL;

    if (fabs(bf->jerk - state.previousJerk) < JERK_MATCH_PRECISION)
    {
        // reuse jerk terms
        bf->cubeRootJerk = state.previousCubeRootJerk;
        bf->reciprocalJerk = state.previousReciprocalJerk;
    }
    else
    {
        bf->cubeRootJerk = cbrt(bf->jerk);
        bf->reciprocalJerk = 1 / bf->jerk;            
        state.previousJerk = bf->jerk;
        state.previousCubeRootJerk = bf->cubeRootJerk;
        state.previousReciprocalJerk = bf->reciprocalJerk;
    }

    // assume continuous path control
    bf->replannable = true;

    bf->entryVMax = bf->cruiseVMax;
    bf->deltaVMax = getTargetVelocity(0, bf->length, bf);
    bf->exitVMax = min(bf->cruiseVMax, (bf->entryVMax + bf->deltaVMax));
    bf->brakingVelocity = bf->deltaVMax;

    // entering running state
    state.motionState = MOTION_RUN;
    
    // replan block list and commit current block
    uint8_t runtimeFlag = false;
    planBlockList(bf, &runtimeFlag);
    PlannerBufferPool::QueueWriteBuffer(MOVE_TYPE_ALINE);
    
    return MC_STATUS_SUCCESS;
}

// Functions for performing holds
//
// Holds work like this:
// 
//   - Hold is initiated by setting hold state to SYNC and motion state to HOLD
//
//   - Hold state == SYNC tells the aline exec routine to execute the next aline 
//     segment then set hold state to PLAN. This gives the planner sufficient 
//     time to replan the block list for the hold before the next aline 
//     segment needs to be processed.
//
//   - Hold state == PLAN tells the planner to replan the runtime buffer, the current
//     run buffer, and any subsequent buffers in the queue as necessary to execute a
//     hold. Hold planning replans the planner buffer queue down to zero and then
//     back up from zero. Hold state is set to DECEL when planning is complete.
//
//   - Hold state == DECEL persists until the aline execution gets runs to 
//     zero velocity, at which point hold state transitions to HOLD.
//
//   - Hold state == HOLD persists until the hold ends via a call to EndHold().
//     The client must wait until the hold state has transitions to HOLD before
//     calling EndHold().
//
//   - Ending the hold sets the hold state to OFF which enables
//     the exec routine to continue processing. Move execution begins with the 
//     first buffer after the hold.
//
// Terms used:
//   - mr is the runtime buffer. It was initially loaded from the bf buffer.
//   - bp+0 is the "companion" bf buffer to the mr buffer.
//   - bp+1 is the bf buffer following bp+0. This runs through bp+N
//   - bp (by itself) just refers to the current buffer being adjusted / replanned
//
// Details:
//   Planning re-uses bp+0 as an "extra" buffer. Normally bp+0 is returned 
//   to the buffer pool as it is redundant once mr is loaded. Use the extra 
//   buffer to split the move in two where the hold decelerates to zero. Use 
//   one buffer to go to zero, the other to replan up from zero. All buffers past
//   that point are unaffected other than that they need to be replanned for velocity.  
//
//  Note:
//     There are multiple opportunities for more efficient organization of 
//     code in this module, but the code is so complicated I just left it
//     organized for clarity and hoped for the best from compiler optimization. 

// Replan buffers to execute hold
// Called from main loop and responds to change in hold state
Status Planner::PlanHoldCallback()
{
    if (state.holdState != HOLD_PLAN) return MC_STATUS_NOOP; // not planning a hold

    // working buffer pointer
    Buffer* bp;

    if ((bp = PlannerBufferPool::GetRunBuffer()) == NULL) return MC_STATUS_NOOP; // nothing is running

    uint8_t runtimeFlag = true;   // used to tell replan to account for mr buffer Vx
    float availableRuntimeLength; // available length left in mr buffer for deceleration
    float brakingVelocity;        // velocity left to shed to brake to zero
    float brakingLength;          // distance required to brake to zero from braking_velocity

    // examine and process mr buffer
    availableRuntimeLength =
        (sqrt(square(state.endpoint[Z_AXIS] - state.position[Z_AXIS]) +
              square(state.endpoint[R_AXIS] - state.position[R_AXIS])));

    brakingVelocity = computeNextSegmentVelocity();
    brakingLength = getTargetLength(brakingVelocity, 0, bp); // bp is OK to use here

    // Hack to prevent Case 2 moves for perfect-fit decels. Happens in homing situations
    // The real fix: The braking velocity cannot simply be the mr.segmentVelocity as this
    // is the velocity of the last segment, not the one that's going to be executed next.
    // The brakingVelocity needs to be the velocity of the next segment that has not yet 
    // been computed. In the mean time, this hack will work. 
    if (brakingLength > availableRuntimeLength && fp_ZERO(bp->exitVelocity))
        brakingLength = availableRuntimeLength;

    // Case 1: deceleration fits entirely in mr
    if (brakingLength <= availableRuntimeLength)
    {
        // set mr to a tail to perform the deceleration
        state.exitVelocity = 0;
        state.tailLength = brakingLength;
        state.cruiseVelocity = brakingVelocity;
        state.moveState = MOVE_STATE_TAIL;
        state.sectionState = MOVE_STATE_NEW;

        // re-use bp+0 to be the hold point and to draw the remaining length
        bp->length = availableRuntimeLength - brakingLength;
        bp->deltaVMax = getTargetVelocity(0, bp->length, bp);
        bp->entryVMax = 0;                  // set bp+0 as hold point
        bp->moveState = MOVE_STATE_NEW;     // tell _exec to re-use the bf buffer

        resetReplannableList();             // make it replan all the blocks
        planBlockList(PlannerBufferPool::GetLastBuffer(), &runtimeFlag);
        state.holdState = HOLD_DECEL;       // set state to decelerate and exit
        return MC_STATUS_SUCCESS;
    }

    // Case 2: deceleration exceeds available length in mr buffer
    // First, replan mr to minimum (but non-zero) exit velocity

    state.moveState = MOVE_STATE_TAIL;
    state.sectionState = MOVE_STATE_NEW;
    state.tailLength = availableRuntimeLength;
    state.cruiseVelocity = brakingVelocity;
    state.exitVelocity = brakingVelocity - getTargetVelocity(0, availableRuntimeLength, bp); 

    // Find the point where deceleration reaches zero. This could span multiple buffers.
    brakingVelocity = state.exitVelocity; // adjust braking velocity downward
    bp->moveState = MOVE_STATE_NEW;       // tell _exec to re-use buffer
    
    // a safety to avoid wraparound
    for (uint8_t i = 0; i < PLANNER_BUFFER_POOL_SIZE; i++)
    {
        PlannerBufferPool::CopyBuffer(bp, bp->next); // copy bp+1 into bp+0 (and onward...)
        
        // skip any non-move buffers
        if (bp->moveType != MOVE_TYPE_ALINE)
        {
            bp = bp->next; // point to next buffer
            continue;
        }
        bp->entryVMax = brakingVelocity; // velocity we need to shed
        brakingLength = getTargetLength(brakingVelocity, 0, bp);

        if (brakingLength > bp->length)
        {
            // decel does not fit in bp buffer
            bp->exitVMax = brakingVelocity - getTargetVelocity(0, bp->length, bp);
            brakingVelocity = bp->exitVMax; // braking velocity for next buffer
            bp = bp->next; // point to next buffer
            continue;
        }
        break;
    }

    // Deceleration now fits in the current bp buffer
    // Plan the first buffer of the pair as the decel, the second as the accel
    bp->length = brakingLength;
    bp->exitVMax = 0;

    bp = bp->next; // point to the acceleration buffer
    bp->entryVMax = 0;
    bp->length -= brakingLength; // the buffers were identical (and hence their lengths)
    bp->deltaVMax = getTargetVelocity(0, bp->length, bp);
    bp->exitVMax = bp->deltaVMax;

    resetReplannableList(); // make it replan all the blocks
    planBlockList(PlannerBufferPool::GetLastBuffer(), &runtimeFlag);
    state.holdState = HOLD_DECEL; // set state to decelerate and exit
    return MC_STATUS_SUCCESS;
}

// Initiate a hold
void Planner::BeginHold()
{
    state.motionState = MOTION_HOLD;
    state.holdState = HOLD_SYNC;
}


// Remove the hold and restart move execution
Status Planner::EndHold()
{
    state.holdState = HOLD_OFF;
    Buffer* bf;
    if ((bf = PlannerBufferPool::GetRunBuffer()) == NULL) // NULL means nothing's running
    {
        state.motionState = MOTION_STOP;
        return MC_STATUS_NOOP;
    }
    state.motionState = MOTION_RUN;
    Motors::RequestMoveExecution();
    return MC_STATUS_SUCCESS;
}

// Update state to reflect move completion
void Planner::EndMove()
{
    state.motionState = MOTION_STOP;
    state.holdState = HOLD_OFF;
    state.segmentVelocity = 0;
}

// Helper function for updating segment velocity
static float computeNextSegmentVelocity()
{
    if (state.moveState == MOVE_STATE_BODY) return state.segmentVelocity;
    return state.segmentVelocity + state.forwardDiff1;
}

// A-Line execution routines
// Everything here fires from LO interrupt and must be interrupt safe
//
// Returns:
//  STAT_OK        move is done
//  STAT_EAGAIN    move is not finished - has more segments to run
//  STAT_NOOP      cause no operation from the steppers - do not load the move
//  STAT_xxxxx     fatal error. Ends the move and frees the bf buffer
// 
// This routine is called from the (LO) interrupt level. The interrupt 
// sequencing relies on the behaviors of the routines being exactly correct.
// Each call to executeALine() must execute and prep *one and only one* 
// segment. If the segment is the not the last segment in the bf buffer the 
// function() must return STAT_EAGAIN. If it's the last segment it must return 
// STAT_OK. If it encounters a fatal error that would terminate the move it 
// should return a valid error code. Failure to obey this will introduce 
// subtle and very difficult to diagnose bugs (trust me on this).
//
// Note 1 Returning STAT_OK ends the move and frees the bf buffer. 
//        Returning STAT_OK at this point does NOT advance position meaning any
//        position error will be compensated by the next move.
//
// Note 2 Solves a potential race condition where the current move ends but the 
//        new move has not started because the previous move is still being run 
//        by the steppers. Planning can overwrite the new move.
//
// Operation:
//   Aline generates jerk-controlled S-curves as per Ed Red's course notes:
//     http://www.et.byu.edu/~ered/ME537/Notes/Ch5.pdf
//     http://www.scribd.com/doc/63521608/Ed-Red-Ch5-537-Jerk-Equations
//
//   A full trapezoid is divided into 5 periods Periods 1 and 2 are the 
//   first and second halves of the acceleration ramp (the concave and convex 
//   parts of the S curve in the "head"). Periods 3 and 4 are the first 
//   and second parts of the deceleration ramp (the tail). There is also 
//   a period for the constant-velocity plateau of the trapezoid (the body).
//   There are various degraded trapezoids possible, including 2 section 
//   combinations (head and tail; head and body; body and tail), and single 
//   sections - any one of the three.
//
//   The equations that govern the acceleration and deceleration ramps are:
//
//     Period 1    V = Vi + Jm*(T^2)/2
//     Period 2    V = Vh + As*T - Jm*(T^2)/2
//     Period 3    V = Vi - Jm*(T^2)/2
//     Period 4    V = Vh + As*T + Jm*(T^2)/2
//
//   These routines play some games with the acceleration and move timing 
//   to make sure this actually all works out. move_time is the actual time of the 
//   move, accel_time is the time valaue needed to compute the velocity - which 
//   takes the initial velocity into account (move_time does not need to).
//
// State transitions:
//
//   bf->moveState transitions:
//    from _NEW to _RUN on first call
//    from _RUN to _OFF on final call
//    or just remains _OFF
//
//   mr.moveState transitions on first call from _OFF to one of _HEAD, _BODY, _TAIL
//   Within each section state may be 
//    _NEW - trigger initialization
//    _RUN1 - run the first part
//    _RUN2 - run the second part 

// Acceleration line main routine
static Status executeALine(Buffer* bf)
{
    uint8_t status = MC_STATUS_SUCCESS;

    if (bf->moveState == MOVE_STATE_OFF) return MC_STATUS_NOOP;

    if (state.moveState == MOVE_STATE_OFF)
    {
        if (state.holdState == HOLD_HOLD) return MC_STATUS_NOOP; // stop here if holding

        // initialization to process the new incoming bf buffer
        bf->replannable = false;

        if (fp_ZERO(bf->length))
        {
            state.moveState = MOVE_STATE_OFF;    // reset mr buffer
            state.sectionState = MOVE_STATE_OFF;
            bf->next->replannable = false;       // prevent overplanning (Note 2)
            Motors::SetNextSegmentNull();        // call this to keep the loader happy
            PlannerBufferPool::FreeRunBuffer();
            return MC_STATUS_NOOP;
        }

        bf->moveState = MOVE_STATE_RUN;
        state.moveState = MOVE_STATE_HEAD;
        state.sectionState = MOVE_STATE_NEW;
        state.jerk = bf->jerk;
        state.headLength = bf->headLength;
        state.bodyLength = bf->bodyLength;
        state.tailLength = bf->tailLength;
        state.entryVelocity = bf->entryVelocity;
        state.cruiseVelocity = bf->cruiseVelocity;
        state.exitVelocity = bf->exitVelocity;
        CopyAxisVector(state.unit, bf->unit);
        CopyAxisVector(state.endpoint, bf->target);     // save the final target of the move
        CopyAxisVector(state.direction, bf->direction);
    }

    // NB: from this point on the contents of the bf buffer do not affect execution

    // main dispatcher to process segments
    switch (state.moveState)
    {
        case MOVE_STATE_HEAD:
            status = executeALineHead();
            break;
        case MOVE_STATE_BODY:
            status = executeALineBody();
            break;
        case MOVE_STATE_TAIL:
            status = executeALineTail();
            break;
        case MOVE_STATE_SKIP:
            status = MC_STATUS_SUCCESS;
            break;
        case MOVE_STATE_OFF:
        case MOVE_STATE_NEW:
        case MOVE_STATE_RUN:
        case MOVE_STATE_RUN2:
            // only concerned with accel/decel/cruise
            break;
    }

    // hold processing - see notes on holds near PlanHoldCallback()
    // catch the hold request and start the planning the hold
    if (state.holdState == HOLD_SYNC) state.holdState = HOLD_PLAN;

    // Look for the end of the decel to go into HOLD state
    if (state.holdState == HOLD_DECEL && status == MC_STATUS_SUCCESS)
    {
        state.holdState = HOLD_HOLD;
        state.motionState = MOTION_HOLD;
    }

    // There are 3 things that can happen here depending on return conditions:
    //    status       bf->moveState   Description
    //    ---------    --------------  ----------------------------------------
    //    EAGAIN       <don't care>    mr buffer has more segments to run
    //    SUCCESS      MOVE_STATE_RUN  mr and bf buffers are done
    //    SUCCESS      MOVE_STATE_NEW  mr done; bf must be run again (it's been reused)

    if (status != MC_STATUS_EAGAIN)
    { 
        state.moveState = MOVE_STATE_OFF;    // reset mr buffer
        state.sectionState = MOVE_STATE_OFF;
        bf->next->replannable = false;       // prevent overplanning (Note 2)

        if (bf->moveState == MOVE_STATE_RUN)
            // free bf if it's actually done
            PlannerBufferPool::FreeRunBuffer();
    }
    return status;
}

// Helper for acceleration section
static Status executeALineHead()
{
    if (state.sectionState == MOVE_STATE_NEW)
    {
        // new move - initialize the runtime buffer
        if (fp_ZERO(state.headLength))
        { 
            state.moveState = MOVE_STATE_BODY;
            return executeALineBody(); // skip ahead to the body generator
        }
        
        state.midpointVelocity = (state.entryVelocity + state.cruiseVelocity) / 2;
        state.moveTime = state.headLength / state.midpointVelocity;           // time for entire accel region
        state.segments = ceil(uSec(state.moveTime) / (2 * NOM_SEGMENT_USEC)); // # of segments in *each half*
        state.segmentMoveTime = state.moveTime / (2 * state.segments);
        state.segmentCount = (uint32_t)state.segments;

        if ((state.microseconds = uSec(state.segmentMoveTime)) < MIN_SEGMENT_USEC)
            // exit without advancing position
            return MC_STATUS_BLOCK_SKIPPED;

        initForwardDiffs(state.entryVelocity, state.midpointVelocity);
        state.sectionState = MOVE_STATE_RUN1;
    }

    if (state.sectionState == MOVE_STATE_RUN1)
    {
        // concave part of accel curve (period 1)
        state.segmentVelocity += state.forwardDiff1;

        if (executeALineSegment(false) == MC_STATUS_COMPLETE)
        {
            // set up for second half
            state.segmentCount = (uint32_t)state.segments;
            state.sectionState = MOVE_STATE_RUN2;

            // Here's a trick: The second half of the S starts at the end of the first,
            //  And the only thing that changes is the sign of mr.forward_diff_2
            state.forwardDiff2 = -state.forwardDiff2;
        }
        else
            state.forwardDiff1 += state.forwardDiff2;
        
        return MC_STATUS_EAGAIN;
    }

    if (state.sectionState == MOVE_STATE_RUN2)
    {
        // convex part of accel curve (period 2)
        state.segmentVelocity += state.forwardDiff1;
        state.forwardDiff1 += state.forwardDiff2;

        if (executeALineSegment(false) == MC_STATUS_COMPLETE)
        {
            if (fp_ZERO(state.bodyLength) && fp_ZERO(state.tailLength)) return MC_STATUS_SUCCESS; // end the move
            state.moveState = MOVE_STATE_BODY;
            state.sectionState = MOVE_STATE_NEW;
        }
    }

    return MC_STATUS_EAGAIN;
}

// Helper for cruise section
// The body is broken into little segments even though it is a straight line so that 
// holds can happen in the middle of a line with a minimum of latency
static Status executeALineBody()
{
    if (state.sectionState == MOVE_STATE_NEW)
    {
        if (fp_ZERO(state.bodyLength))
        {
            state.moveState = MOVE_STATE_TAIL;
            mcState->decelerationStarted = true;
            return executeALineTail(); // skip ahead to tail periods
        }
        
        state.moveTime = state.bodyLength / state.cruiseVelocity;
        state.segments = ceil(uSec(state.moveTime) / NOM_SEGMENT_USEC);
        state.segmentMoveTime = state.moveTime / state.segments;
        state.segmentVelocity = state.cruiseVelocity;
        state.segmentCount = (uint32_t)state.segments;
        
        if ((state.microseconds = uSec(state.segmentMoveTime)) < MIN_SEGMENT_USEC)
            return MC_STATUS_BLOCK_SKIPPED; // exit without advancing position

        state.sectionState = MOVE_STATE_RUN;
    }
    
    if (state.sectionState == MOVE_STATE_RUN)
    {
        // straight part (period 3)
        if (executeALineSegment(false) == MC_STATUS_COMPLETE)
        {
            if (fp_ZERO(state.tailLength)) return MC_STATUS_SUCCESS; // end the move
            state.moveState = MOVE_STATE_TAIL;
            mcState->decelerationStarted = true;
            state.sectionState = MOVE_STATE_NEW;
        }
    }

    return MC_STATUS_EAGAIN;
}

// Helper for deceleration section
static Status executeALineTail()
{
    if (state.sectionState == MOVE_STATE_NEW)
    {
        if (fp_ZERO(state.tailLength)) return MC_STATUS_SUCCESS; // end the move

        state.midpointVelocity = (state.cruiseVelocity + state.exitVelocity) / 2;
        state.moveTime = state.tailLength / state.midpointVelocity;
        state.segments = ceil(uSec(state.moveTime) / (2 * NOM_SEGMENT_USEC)); // # of segments in *each half*
        state.segmentMoveTime = state.moveTime / (2 * state.segments);        // time to advance for each segment
        state.segmentCount = (uint32_t)state.segments;

        if ((state.microseconds = uSec(state.segmentMoveTime)) < MIN_SEGMENT_USEC)
            return MC_STATUS_BLOCK_SKIPPED; // exit without advancing position

        initForwardDiffs(state.cruiseVelocity, state.midpointVelocity);
        state.sectionState = MOVE_STATE_RUN1;
    }

    if (state.sectionState == MOVE_STATE_RUN1)
    {
        // convex part (period 4)
        state.segmentVelocity += state.forwardDiff1;
        if (executeALineSegment(false) == MC_STATUS_COMPLETE)
        {
            // set up for second half
            state.segmentCount = (uint32_t)state.segments;
            state.sectionState = MOVE_STATE_RUN2;

            // Here's a trick: The second half of the S starts at the end of the first,
            //  And the only thing that changes is the sign of forwardDiff2
            state.forwardDiff2 = -state.forwardDiff2;
        }
        else
            state.forwardDiff1 += state.forwardDiff2;

        return MC_STATUS_EAGAIN;
    }

    if (state.sectionState == MOVE_STATE_RUN2)
    {
        // concave part (period 5)
        state.segmentVelocity += state.forwardDiff1;
        state.forwardDiff1 += state.forwardDiff2;
        if (executeALineSegment(true) == MC_STATUS_COMPLETE) return MC_STATUS_SUCCESS; // end the move
    }

    return MC_STATUS_EAGAIN;
}

// Helper for running a segment
static Status executeALineSegment(uint8_t correctionFlag)
{
    float steps[AXES_COUNT];

    // Multiply computed length by the unit vector to get the contribution for
    // each axis. Set the target in absolute coords and compute relative steps.

    if (correctionFlag == true && state.segmentCount == 1 && state.motionState == MOTION_RUN)
    {
        // rounding error correction for last segment
        state.target[Z_AXIS] = state.endpoint[Z_AXIS];
        state.target[R_AXIS] = state.endpoint[R_AXIS];
    }
    else
    {
        float intermediate = state.segmentVelocity * state.segmentMoveTime;
        state.target[Z_AXIS] = state.position[Z_AXIS] + (state.unit[Z_AXIS] * intermediate);
        state.target[R_AXIS] = state.position[R_AXIS] + (state.unit[R_AXIS] * intermediate);
    }

    // Determine step pulse count
    steps[Z_AXIS] = state.pulsesPerUnit[Z_AXIS] * (state.target[Z_AXIS] - state.position[Z_AXIS]);
    steps[R_AXIS] = state.pulsesPerUnit[R_AXIS] * (state.target[R_AXIS] - state.position[R_AXIS]);

    if (Motors::SetNextSegment(steps, state.direction, state.microseconds) == MC_STATUS_SUCCESS)
        CopyAxisVector(state.position, state.target); // update runtime position  

    if (--state.segmentCount == 0)
        return MC_STATUS_COMPLETE; // this section has run all its segments

    return MC_STATUS_EAGAIN; // this section still has more segments to run
}

// Forward difference math explained:
//   We're using two quadratic curves end-to-end, forming the concave and convex 
//   section of the s-curve. For each half, we have three points:
//
//    T[0] is the start point, or the entro or middle of the "s". This will be one of:
//      - entry_velocity (acceleration concave),
//      - cruise_velocity (deceleration concave), or
//      - midpoint_velocity (convex)
//    T[1] is the "control point" set to T[0] for concave sections, and T[2] for convex
//    T[2] is the end point of the quadratic, which will be the midpoint or endpoint of the s.
//
//    A = T[0] - 2*T[1] + T[2]
//    B = 2 * (T[1] - T[0])
//    C = T[0]
//    h = (1/mr.segments)
//
//  forwardDiff1 = Ah^2+Bh = (T[0] - 2*T[1] + T[2])h*h + (2 * (T[1] - T[0]))h
//  forwardDiff2 = 2Ah^2 = 2*(T[0] - 2*T[1] + T[2])h*h
//
//  NOTE: t1 will always be == t0, so we don't pass it
static void initForwardDiffs(float t0, float t2)
{
    float HSquared = square(1 / state.segments);

    // A = T[0] - 2*T[1] + T[2], if T[0] == T[1], then it becomes - T[0] + T[2]
    float AHSquared = (t2 - t0) * HSquared;

    // Ah²+Bh, and B=2 * (T[1] - T[0]), if T[0] == T[1], then it becomes simply Ah^2
    state.forwardDiff1 = AHSquared;
    state.forwardDiff2 = 2 * AHSquared;
    state.segmentVelocity = t0;
}

// This set of functions returns the fourth thing knowing the other three.
//  
//    Jm = the given maximum jerk
//    T  = time of the entire move
//    T  = 2*sqrt((Vt-Vi)/Jm)
//    As = The acceleration at inflection point between convex and concave portions of the S-curve.
//    As = (Jm*T)/2
//    Ar = ramp acceleration
//    Ar = As/2 = (Jm*T)/4
//  
// Assumes Vt, Vi and L are positive or zero
// Cannot assume Vt>=Vi due to rounding errors and use of PLANNER_VELOCITY_TOLERANCE
// necessitating the introduction of fabs()
//
// getTargetLength() is a convenient function for determining the 
// optimal length (L) of a line given the inital velocity (Vi), 
// target velocity (Vt) and maximum jerk (Jm).
//
// The length (distance) equation is derived from: 
//
//  a) L = (Vt-Vi) * T - (Ar*T^2)/2    ... which becomes b) with substitutions for Ar and T
//  b) L = (Vt-Vi) * 2*sqrt((Vt-Vi)/Jm) - (2*sqrt((Vt-Vi)/Jm) * (Vt-Vi))/2
//  c) L = (Vt-Vi)^(3/2) / sqrt(Jm)    ...is an alternate form of b) (see Wolfram Alpha)
//  c')L = (Vt-Vi) * sqrt((Vt-Vi)/Jm) ... second alternate form; requires Vt >= Vi
//
//  Notes: Ar = (Jm*T)/4                   Ar is ramp acceleration
//         T  = 2*sqrt((Vt-Vi)/Jm)         T is time
//         Assumes Vt, Vi and L are positive or zero
//         Cannot assume Vt>=Vi due to rounding errors and use of PLANNER_VELOCITY_TOLERANCE
//           necessitating the introduction of fabs()
//
// getTargetVelocity() is a convenient function for determining Vt target 
// velocity for a given the initial velocity (Vi), length (L), and maximum jerk (Jm).
// Equation d) is b) solved for Vt. Equation e) is c) solved for Vt. Use e) (obviously)
//
//  d) Vt = (sqrt(L)*(L/sqrt(1/Jm))^(1/6)+(1/Jm)^(1/4)*Vi)/(1/Jm)^(1/4)
//  e) Vt = L^(2/3) * Jm^(1/3) + Vi
//
// FYI: Here's an expression that gives the jerk for a given deltaV and L:
// cube(deltaV / (pow(L, 0.66666666)));

// Derive accel/decel length from delta V and jerk
static float getTargetLength(float Vi, float Vt, const Buffer* bf)
{
    return fabs(Vi - Vt) * sqrt(fabs(Vi - Vt) * bf->reciprocalJerk);
}

// Derive velocity achievable from delta V and length
static float getTargetVelocity(float Vi, float L, const Buffer* bf)
{
    return pow(L, 0.66666666) * bf->cubeRootJerk + Vi;
}

// Plan the entire block list
//
// Plans all blocks between and including the first block and the block provided (bf).
// Sets entry, exit and cruise v's from vmax's then calls trapezoid generation. 
//
// Variables that must be provided in the mpBuffers that will be processed:
//
//   bf (function arg)     - end of block list (last block in time)
//   bf->replannable       - start of block list set by last FALSE value [Note 1]
//   bf->moveType          - typically ALINE. Other move_types should be set to 
//                           length=0, entryVMax=0 and exitVMax=0 and are treated
//                           as a momentary hold (plan to zero and from zero).
//
//   bf->length            - provides block length
//   bf->entryVMax         - used during forward planning to set entry velocity
//   bf->cruiseVMax        - used during forward planning to set cruise velocity
//   bf->exitVMax          - used during forward planning to set exit velocity
//   bf->deltaVMax         - used during forward planning to set exit velocity
//
//   bf->reciprocalJerk    - used during trapezoid generation
//   bf->cubeRootJerk      - used during trapezoid generation
//
// Variables that will be set during processing:
//
//   bf->replannable       - set if the block becomes optimally planned
//
//   bf->brakingVelocity   - set during backward planning
//   bf->entryVelocity     - set during forward planning
//   bf->cruiseVelocity    - set during forward planning
//   bf->exitVelocity      - set during forward planning
//
//   bf->headLength        - set during trapezoid generation
//   bf->bodyLength        - set during trapezoid generation
//   bf->tailLength        - set during trapezoid generation
//
// Variables that are ignored but here's what you would expect them to be:
//
//   bf->moveState         - NEW for all blocks but the earliest
//   bf->target[]          - block target position
//   bf->unit[]            - block unit vector
//   bf->time              - gets set later
//   bf->jerk              - source of the other jerk variables. Used in mr.
// 
// Notes:
//  [1] Whether or not a block is planned is controlled by the bf->replannable 
//      setting (set TRUE if it should be). Replan flags are checked during the 
//      backwards pass and prune the replan list to include only the the latest 
//      blocks that require planning
//
//      In normal operation the first block (currently running block) is not 
//      replanned, but may be for feedholds and feed overrides. In these cases 
//      the prep routines modify the contents of the mr buffer and re-shuffle 
//      the block list, re-enlisting the current bf buffer with new parameters.
//      These routines also set all blocks in the list to be replannable so the 
//      list can be recomputed regardless of exact stops and previous replanning 
//      optimizations.
static void planBlockList(Buffer* bf, uint8_t* runtimeFlag)
{
    Buffer* bp = bf;

    // Backward planning pass. Find beginning of the list and update the braking velocities.
    // At the end *bp points to the first buffer before the list.
    while ((bp = bp->previous) != bf)
    {
        if (bp->replannable == false) break;
        bp->brakingVelocity = min(bp->next->entryVMax, bp->next->brakingVelocity) + bp->deltaVMax;
    }

    // forward planning pass - recomputes trapezoids in the list.
    while ((bp = bp->next) != bf)
    {
        if (bp->previous == bf || *runtimeFlag == true)
        {
            bp->entryVelocity = bp->entryVMax; // first block in the list
            *runtimeFlag = false;
        }
        else
            bp->entryVelocity = bp->previous->exitVelocity; // other blocks in the list
        
        bp->cruiseVelocity = bp->cruiseVMax;
        bp->exitVelocity = Min4(bp->exitVMax, bp->next->brakingVelocity, bp->next->entryVMax,
                (bp->entryVelocity + bp->deltaVMax));

        calculateTrapezoid(bp);

        // test for optimally planned trapezoids - only need to check various exit conditions
        if (bp->exitVelocity == bp->exitVMax || bp->exitVelocity == bp->next->entryVMax || 
                (bp->previous->replannable == false && bp->exitVelocity == bp->entryVelocity + bp->deltaVMax))
            bp->replannable = false;
    }

    // finish up the last block move
    bp->entryVelocity = bp->previous->exitVelocity;
    bp->cruiseVelocity = bp->cruiseVMax;
    bp->exitVelocity = 0;
    calculateTrapezoid(bp);
}

// Resets all blocks in the planning list to be replannable
static void resetReplannableList()
{
    Buffer* bf = PlannerBufferPool::GetFirstBuffer();
    
    if (bf == NULL) return;

    Buffer* bp = bf;
    do
    {
        bp->replannable = true;
    }
    while ((bp = bp->next) != bf && bp->moveState != MOVE_STATE_OFF);
}

// Calculate trapezoid parameters
//
// This rather brute-force function sets section lengths and velocities based 
// on the line length and velocities requested. It modifies the bf buffer and 
// returns accurate head_length, body_length and tail_length, and accurate or 
// reasonably approximate velocities. We care about accuracy on lengths, less 
// so for velocity (as long as velocity err's on the side of too slow). We need 
// the velocities to be set even for zero-length sections so we can compute 
// entry and exits for adjacent sections.
//
// Inputs used are:
//   bf->length            - actual block length (must remain accurate)
//   bf->entryVelocity     - requested Ve
//   bf->cruiseVelocity    - requested Vt
//   bf->exitVelocity      - requested Vx
//   bf->cruiseVMax        - used in some comparisons
//
// Variables set may include the velocities above (not the vmax), and:
//   bf->headLength        - bf->length allocated to head
//   bf->bodyLength        - bf->length allocated to body
//   bf->tailLength        - bf->length allocated to tail
//
// Note: The following condition must be met on entry: Ve <= Vt >= Vx 
//
// Classes of moves:
//   Maximum-Fit - The trapezoid can accommodate its maximum velocity values for
//     the given length (entry_vmax, cruise_vmax, exit_vmax). But the trapezoid 
//     generator actally doesn't know about the max's and only processes requested 
//     values.
//
//   Requested-Fit - The move has sufficient length to achieve the target ("set") 
//     cruising velocity. It will accommodate the acceleration / deceleration 
//     profile and in the distance given (length)
//
//   Rate-Limited-Fit - The move does not have sufficient length to achieve target 
//     cruising velocity - the target velocity will be lower than the requested 
//     velocity. The entry and exit velocities are satisfied. 
//
//   Degraded-Fit - The move does not have sufficient length to transition from
//     the entry velocity to the exit velocity in the available length. These 
//     velocities are not negotiable, so a degraded solution is found.
//
//   No-Fit - The move cannot be executed as the planned execution time is less
//     than the minimum segment interpolation time of the runtime execution module.
//
// Various cases handled;
//   No-Fit cases - the line is too short to plan
//     No fit
//
//   Degraded fit cases - line is too short to satisfy both Ve and Vx
//     H"  Ve<Vx       Ve is degraded (velocity step). Vx is met
//     T"  Ve>Vx       Ve is degraded (velocity step). Vx is met
//     B   <short>     line is very short but drawable; is treated as a body only
//
//   Rate-Limited cases - Ve and Vx can be satisfied but Vt cannot
//     HT  (Ve=Vx)<Vt  symmetric case. Split the length and compute Vt.
//     HT' (Ve!=Vx)<Vt asymmetric case. Find H and T by successive approximation.
//     HBT'            Lb < min body length - treated as an HT case
//     H'              Lb < min body length - reduce J to fit H to length
//     T'              Lb < min body length - reduce J to fit T to length
//
//   Requested-Fit cases
//     HBT Ve<Vt>Vx    sufficient length exists for all part (corner case: HBT')
//     HB  Ve<Vt=Vx    head accelerates to cruise - exits at full speed (corner case: H')
//     BT  Ve=Vt>Vx    enter at full speed and decelerate (corner case: T')
//     HT  Ve & Vx     perfect fit HT (very rare)
//     H   Ve<Vx       perfect fit H (common, results from planning)
//     T   Ve>Vx       perfect fit T (common, results from planning)
//     B   Ve=Vt=Vx    Velocities tested to tolerance
//
// The order of the cases/tests in the code is pretty important
//
// The minimum lengths are dynamic, and depend on the velocity
// These expressions evaluate to the minimum lengths for the current velocity settings
// Note: The head and tail lengths are 2 minimum segments, the body is 1 min segment
#define MIN_HEAD_LENGTH (MIN_SEGMENT_TIME * (bf->cruiseVelocity + bf->entryVelocity))
#define MIN_TAIL_LENGTH (MIN_SEGMENT_TIME * (bf->cruiseVelocity + bf->exitVelocity))
#define MIN_BODY_LENGTH (MIN_SEGMENT_TIME * bf->cruiseVelocity)

static void calculateTrapezoid(Buffer* bf) 
{
    // initialize the lengths
    bf->headLength = 0;
    bf->bodyLength = 0;
    bf->tailLength = 0;

    // Combined short cases:
    //  - H and T requested-fit cases (exact fit cases, to within TRAPEZOID_LENGTH_FIT_TOLERANCE)
    //  - H" and T" degraded-fit cases
    //  - H' and T' requested-fit cases where the body residual is less than MIN_BODY_LENGTH
    //  - no-fit case
    // Also converts 2 segment heads and tails that would be too short to a body-only move (1 segment)
    float minimumLength = getTargetLength(bf->entryVelocity, bf->exitVelocity, bf);

    if (bf->length <= minimumLength + MIN_BODY_LENGTH)
    {
        // Head & tail cases
        if (bf->entryVelocity > bf->exitVelocity)
        {
            // Tail cases
            if (bf->length < minimumLength - TRAPEZOID_LENGTH_FIT_TOLERANCE)
                // T" (degraded case)
                bf->entryVelocity = getTargetVelocity(bf->exitVelocity, bf->length, bf);

            bf->cruiseVelocity = bf->entryVelocity;

            if (bf->length >= MIN_TAIL_LENGTH)
                // run this as a 2+ segment tail
                bf->tailLength = bf->length;
            else if (bf->length > MIN_BODY_LENGTH)
                // run this as a 1 segment body
                bf->bodyLength = bf->length;
            else
                bf->moveState = MOVE_STATE_SKIP; // tell runtime to skip the block
            
            return;
        }

        if (bf->entryVelocity < bf->exitVelocity)
        {
            // Head cases
            if (bf->length < minimumLength - TRAPEZOID_LENGTH_FIT_TOLERANCE)
                // H" (degraded case)
                bf->exitVelocity = getTargetVelocity(bf->entryVelocity, bf->length, bf);

            bf->cruiseVelocity = bf->exitVelocity;

            if (bf->length >= MIN_HEAD_LENGTH)
                // run this as a 2+ segment head
                bf->headLength = bf->length;
            else if (bf->length > MIN_BODY_LENGTH)
                // run this as a 1 segment body
                bf->bodyLength = bf->length;
            else
                bf->moveState = MOVE_STATE_SKIP; // tell runtime to skip the block
            
            return;
        }
    }

    // Set head and tail lengths
    bf->headLength = getTargetLength(bf->entryVelocity, bf->cruiseVelocity, bf);
    bf->tailLength = getTargetLength(bf->exitVelocity, bf->cruiseVelocity, bf);
    if (bf->headLength < MIN_HEAD_LENGTH) bf->headLength = 0;
    if (bf->tailLength < MIN_TAIL_LENGTH) bf->tailLength = 0;

    // Rate-limited HT and HT' cases
    if (bf->length < bf->headLength + bf->tailLength)
    {
        // it's rate limited

        // Rate-limited HT case (symmetric case)
        if (fabs(bf->entryVelocity - bf->exitVelocity) < TRAPEZOID_VELOCITY_TOLERANCE)
        {
            bf->headLength = bf->length / 2;
            bf->tailLength = bf->headLength;
            bf->cruiseVelocity = min(bf->cruiseVMax, getTargetVelocity(bf->entryVelocity, bf->headLength, bf));
            return;
        }

        // Rate-limited HT' case (asymmetric) - this is relatively expensive but it's not called very often
        float computedVelocity = bf->cruiseVMax;
        do
        {
            bf->cruiseVelocity = computedVelocity; // initialize from previous iteration 
            bf->headLength = getTargetLength(bf->entryVelocity, bf->cruiseVelocity, bf);
            bf->tailLength = getTargetLength(bf->exitVelocity, bf->cruiseVelocity, bf);

            if (bf->headLength > bf->tailLength)
            {
                bf->headLength = (bf->headLength / (bf->headLength + bf->tailLength)) * bf->length;
                computedVelocity = getTargetVelocity(bf->entryVelocity, bf->headLength, bf);
            }
            else
            {
                bf->tailLength = (bf->tailLength / (bf->headLength + bf->tailLength)) * bf->length;
                computedVelocity = getTargetVelocity(bf->exitVelocity, bf->tailLength, bf);
            }
        }
        while ((fabs(bf->cruiseVelocity - computedVelocity) / computedVelocity) > TRAPEZOID_ITERATION_ERROR_PERCENT);

        bf->cruiseVelocity = computedVelocity;
        bf->headLength = getTargetLength(bf->entryVelocity, bf->cruiseVelocity, bf);
        bf->tailLength = bf->length - bf->headLength;

        if (bf->headLength < MIN_HEAD_LENGTH)
        {
            bf->tailLength = bf->length; // adjust the move to be all tail...
            bf->headLength = 0;          // adjust the jerk to fit to the adjusted length
        }

        if (bf->tailLength < MIN_TAIL_LENGTH)
        {
            bf->headLength = bf->length; //...or all head
            bf->tailLength = 0;
        }

        return;
    }

    // Requested-fit cases: remaining of: HBT, HB, BT, BT, H, T, B, cases
    bf->bodyLength = bf->length - bf->headLength - bf->tailLength;

    // If a non-zero body is < minimum length distribute it to the head and/or tail
    // This will generate small (acceptable) velocity errors in runtime execution
    // but preserve correct distance, which is more important.
    if (bf->bodyLength < MIN_BODY_LENGTH && fp_NOT_ZERO(bf->bodyLength))
    {
        if (fp_NOT_ZERO(bf->headLength))
        {
            if (fp_NOT_ZERO(bf->tailLength))
            {
                // HBT reduces to HT
                bf->headLength += bf->bodyLength/2;
                bf->tailLength += bf->bodyLength/2;
            }
            else
                // HB reduces to H
                bf->headLength += bf->bodyLength;

        }
        else
            // BT reduces to T
            bf->tailLength += bf->bodyLength;
        
        bf->bodyLength = 0;

        // If the body is a standalone make the cruise velocity match the entry velocity 
        // This removes a potential velocity discontinuity at the expense of top speed
    }
    else if (fp_ZERO(bf->headLength) && fp_ZERO(bf->tailLength))
        bf->cruiseVelocity = bf->entryVelocity;
}
