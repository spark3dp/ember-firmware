/*
 * PlanningBuffer.h
 * Author: Jason Lefley
 * Date  : 2015-06-14
 */

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

struct Buffer
{
    Buffer* previous;                        // static pointer to previous buffer
    Buffer* next;                            // static pointer to next buffer
    Status (*executionFunction)(Buffer *bf); // callback to buffer exec function
    BufferState state;                       // used to manage queueing/dequeueing
    MoveType moveType;                       // used to dispatch to run routine
    MoveState moveState;                     // move state machine sequence
    uint8_t replannable;                     // TRUE if move can be replanned

    float target[AXES_COUNT];                // target position in floating point
    float unit[AXES_COUNT];                  // unit vector for axis scaling & planning
    uint8_t directions[AXES_COUNT];          // movement directions

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

//#define GetNextBuffer(b) ((Buffer*)(b->next))

namespace PlannerBufferPool
{
Status ExecuteRunBuffer();
void Initialize();
void ClearBuffer(Buffer* bf); 
void CopyBuffer(Buffer* bf, const Buffer* bp);
void QueueWriteBuffer(MoveType moveType);
void FreeRunBuffer();
Buffer* GetWriteBuffer(void); 
Buffer* GetRunBuffer(void);
Buffer* GetFirstBuffer(void);
Buffer* GetLastBuffer(void);
};

#endif /* PLANNERBUFFERPOOL_H */
