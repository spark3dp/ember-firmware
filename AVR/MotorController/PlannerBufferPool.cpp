//  File: PlannerBufferPool.cpp
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

#include <string.h>

#include "PlannerBufferPool.h"
#include "Motors.h"

// Increment buffer, wrapping around to head if necessary
#define _bump(a) ((a<PLANNER_BUFFER_POOL_SIZE-1)?(a+1):0)

// Ring buffer to hold planning buffers
struct BufferPool
{
    uint8_t availableBuffers;              // running count of available buffers
    Buffer* writeBuffer;                   // get_write_buffer pointer
    Buffer* queuedWriteBuffer;              // queue_write_buffer pointer
    Buffer* runBuffer;                     // get/end_run_buffer pointer
    Buffer pool[PLANNER_BUFFER_POOL_SIZE]; // buffer storage
};

static BufferPool bufferPool;

// Execute runtime functions to prep move for steppers
Status PlannerBufferPool::ExecuteRunBuffer()
{
    Buffer* bf;

    if ((bf = GetRunBuffer()) == NULL) return MC_STATUS_NOOP; // NULL means nothing's running

    // run the move callback in the planner buffer
    if (bf->executionFunction != NULL)
        return bf->executionFunction(bf);

    return MC_STATUS_INTERNAL_ERROR; // never supposed to get here
}

// Initializes or reset buffers
void PlannerBufferPool::Initialize()
{
    Buffer* pv;

    // zero buffer pool
    memset(&bufferPool, 0, sizeof(bufferPool));
    
    // initialize write and read buffer pointers
    bufferPool.writeBuffer = &bufferPool.pool[0];
    bufferPool.queuedWriteBuffer = &bufferPool.pool[0];
    bufferPool.runBuffer = &bufferPool.pool[0];

    // setup ring pointers
    pv = &bufferPool.pool[PLANNER_BUFFER_POOL_SIZE-1];
    for (uint8_t i = 0; i < PLANNER_BUFFER_POOL_SIZE; i++)
    {
        bufferPool.pool[i].next = &bufferPool.pool[_bump(i)];
        bufferPool.pool[i].previous = pv;
        pv = &bufferPool.pool[i];
    }

    bufferPool.availableBuffers = PLANNER_BUFFER_POOL_SIZE;
}

// Zeroes the contents of the buffer
void PlannerBufferPool::ClearBuffer(Buffer* bf) 
{
    // save pointers
    Buffer* nx = bf->next;
    Buffer* pv = bf->previous;
    
    memset(bf, 0, sizeof(Buffer));

    // restore pointers
    bf->next = nx;
    bf->previous = pv;
}

//  Copies the contents of bp into bf - preserves links
void PlannerBufferPool::CopyBuffer(Buffer* bf, const Buffer* bp)
{
    // save pointers
    Buffer* nx = bf->next;
    Buffer* pv = bf->previous;

    memcpy(bf, bp, sizeof(Buffer));

    // restore pointers
    bf->next = nx;
    bf->previous = pv;
}

// Commit the next write buffer to the queue
// Advances write pointer & changes buffer state
void PlannerBufferPool::QueueWriteBuffer(MoveType moveType)
{
    bufferPool.queuedWriteBuffer->moveType = moveType;
    bufferPool.queuedWriteBuffer->moveState = MOVE_STATE_NEW;
    bufferPool.queuedWriteBuffer->state = BUFFER_STATE_QUEUED;

    // advance the queued buffer pointer
    bufferPool.queuedWriteBuffer = bufferPool.queuedWriteBuffer->next;

    // request a move exec if not busy
    Motors::RequestMoveExecution();
}

// Release the run buffer & return to buffer pool
void PlannerBufferPool::FreeRunBuffer()
{
    // clear it out (& reset replannable)
    ClearBuffer(bufferPool.runBuffer);

    // advance to next run buffer
    bufferPool.runBuffer = bufferPool.runBuffer->next;

    if (bufferPool.runBuffer->state == BUFFER_STATE_QUEUED)
        // pend next buffer only if queued
        bufferPool.runBuffer->state = BUFFER_STATE_PENDING;

    // update count 
    bufferPool.availableBuffers++;
}

// Get pointer to next available write buffer
// Returns pointer or NULL if no buffer available.
Buffer* PlannerBufferPool::GetWriteBuffer()
{
    if (bufferPool.writeBuffer->state == BUFFER_STATE_EMPTY)
    {
        Buffer* w = bufferPool.writeBuffer;

        // save pointers
        Buffer* nx = bufferPool.writeBuffer->next;
        Buffer* pv = bufferPool.writeBuffer->previous;

        memset(bufferPool.writeBuffer, 0, sizeof(Buffer));
        
        // restore pointers
        w->next = nx;
        w->previous = pv;

        w->state = BUFFER_STATE_LOADING;
        bufferPool.availableBuffers--;
        bufferPool.writeBuffer = w->next;
        return w;
    }

    return NULL;
}

// Get pointer to the next or current run buffer
// Returns a new run buffer if prev buf was ENDed
// Returns same buf if called again before ENDing
// Returns NULL if no buffer available
// The behavior supports continuations (iteration)
Buffer* PlannerBufferPool::GetRunBuffer() 
{
    // condition: fresh buffer; becomes running if queued or pending
    if (bufferPool.runBuffer->state == BUFFER_STATE_QUEUED ||
            bufferPool.runBuffer->state == BUFFER_STATE_PENDING)
        bufferPool.runBuffer->state = BUFFER_STATE_RUNNING;

    // condition: asking for the same run buffer for the Nth time
    if (bufferPool.runBuffer->state == BUFFER_STATE_RUNNING)
        // return same buffer
        return bufferPool.runBuffer;

    // condition: no queued buffers
    return NULL;
}

// Returns pointer to first buffer, i.e. the running block
Buffer* PlannerBufferPool::GetFirstBuffer()
{
    return GetRunBuffer();
}

// Returns pointer to last buffer, i.e. last block (zero)
Buffer* PlannerBufferPool::GetLastBuffer()
{
    Buffer* bf = GetRunBuffer();
    Buffer* bp = bf;

    if (bf == NULL) return NULL;

    do
    {
        if (bp->next->moveState == MOVE_STATE_OFF || bp->next == bf) return bp; 
    }
    while ((bp = bp->next) != bf);

    return bp;
}
