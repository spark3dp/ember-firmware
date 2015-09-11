//  File: PlannerBufferPool.h
//  Buffer to hold movement blocks for planning
//  For more information see planner.c and planner.h from TinyG
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

#ifndef PLANNERBUFFERPOOL_H
#define PLANNERBUFFERPOOL_H

#include "MachineDefinitions.h"
#include "Status.h"

#define PLANNER_BUFFER_POOL_SIZE 8

enum MoveType
{                
    MOVE_TYPE_NULL = 0, // null move - does a no-op
    MOVE_TYPE_ALINE     // acceleration planned line
};

enum BufferState
{ 
    BUFFER_STATE_EMPTY = 0, // struct is available for use (MUST BE 0)
    BUFFER_STATE_LOADING,   // being written ("checked out")
    BUFFER_STATE_QUEUED,    // in queue
    BUFFER_STATE_PENDING,   // marked as the next buffer to run
    BUFFER_STATE_RUNNING    // current running buffer
};

enum MoveState
{
    MOVE_STATE_OFF = 0, // move inactive (MUST BE ZERO)
    MOVE_STATE_NEW,     // general value if you need an initialization
    MOVE_STATE_RUN,     // general run state (for non-acceleration moves) 
    MOVE_STATE_RUN2,    // used for sub-states
    MOVE_STATE_HEAD,    // aline() acceleration portions
    MOVE_STATE_BODY,    // aline() cruise portions
    MOVE_STATE_TAIL,    // aline() deceleration portions
    MOVE_STATE_SKIP     // mark a skipped block
};

// Buffer elements for planning moves
// See comments near planBlockList in Planner.cpp for more
// explanation on variables
struct Buffer
{
    Buffer* previous;                        // static pointer to previous buffer
    Buffer* next;                            // static pointer to next buffer
    Status (*executionFunction)(Buffer* bf); // callback to buffer exec function
    BufferState state;                       // used to manage queueing/dequeueing
    MoveType moveType;                       // used to dispatch to run routine
    MoveState moveState;                     // move state machine sequence
    uint8_t replannable;                     // TRUE if move can be replanned

    float target[AXES_COUNT];                // target position in floating point
    float unit[AXES_COUNT];                  // unit vector for axis scaling & planning
    uint8_t direction[AXES_COUNT];           // vector containing flags to indicate movement direction

    float headLength;
    float bodyLength;
    float tailLength;
    float length;                            // total length of line
    
    float entryVelocity;                     // entry velocity requested for the move
    float cruiseVelocity;                    // cruise velocity requested & achieved
    float exitVelocity;                      // exit velocity requested for the move

    float entryVMax;                         // max junction velocity at entry of this move
    float cruiseVMax;                        // max cruise velocity requested for move
    float exitVMax;                          // max exit velocity possible (redundant)
    float deltaVMax;                         // max velocity difference for this move
    float brakingVelocity;                   // current value for braking velocity

    float jerk;                              // maximum linear jerk term for this move
    float reciprocalJerk;                    // 1/Jm used for planning (compute-once)
    float cubeRootJerk;                      // cube root of Jm used for planning (compute-once)
};  

namespace PlannerBufferPool
{
Status ExecuteRunBuffer();
void Initialize();
void ClearBuffer(Buffer* bf); 
void CopyBuffer(Buffer* bf, const Buffer* bp);
void QueueWriteBuffer(MoveType moveType);
void FreeRunBuffer();
Buffer* GetWriteBuffer(); 
Buffer* GetRunBuffer();
Buffer* GetFirstBuffer();
Buffer* GetLastBuffer();
};

#endif  // PLANNERBUFFERPOOL_H
