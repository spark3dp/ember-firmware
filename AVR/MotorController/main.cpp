//  File: main.cpp
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

#include <avr/interrupt.h>

#include "I2CInterface.h"
#include "Hardware.h"
#include "CommandBuffer.h"
#include "MotorController.h"
#include "CommandMap.h"
#include "StateMachine.h"
#include "Planner.h"

#ifdef DEBUG
#include "Debug.h"
#include <avr/pgmspace.h>
#endif  // DEBUG

MotorController_t mcState; // Instance of the global state struct, all members initialized to 0
CommandBuffer commandBuffer;
volatile bool limitSwitchHit;

// Check limit switch interrupt flag and raise limit reached event if set
static void QueryLimitSwitchInterrupt()
{
    if (limitSwitchHit)
    {
        limitSwitchHit = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, AxisLimitReached);
    }
}

// Check for incoming I2C command and raise appropriate event if new command exists in buffer
static void QueryCommandBuffer()
{
    if (!commandBuffer.IsEmpty())
    {
        Command command;
        EventData eventData;

        commandBuffer.GetCommand(command);
        uint8_t eventCode = CommandMap::GetEventCode(command.Register(), command.Action());

#ifdef DEBUG
        printf_P(PSTR("INFO: Received i2c message: register %2x, command: %d, value: %ld, event code: %d\n"),
                command.Register(), command.Action(), command.Parameter(), eventCode);
#endif

        if (eventCode == 0)
        {
#ifdef DEBUG
            printf_P(PSTR("ERROR: Command does not have corresponding state machine event, not handling\n"));
#endif
            mcState.error = true;
            mcState.status = MC_STATUS_COMMAND_UNKNOWN;
            return;
        }

        eventData.command = command.Action();
        eventData.parameter = command.Parameter();
        MotorController_State_Machine_Event(&mcState, eventData, eventCode);
    }
}

// Check motion complete flag and raise motion complete event if set
static void QueryMotionComplete()
{
    if (mcState.motionComplete)
    {
        mcState.motionComplete = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, MotionComplete);
    }
}

// Check for dequeued events
// Returns true if a dequeued event found
// The return value allows the main loop to check for additional queued events before
// checking for new commands
static bool QueryEventQueue()
{
    if (mcState.queuedEvent)
    {
        mcState.queuedEvent = false;
        MotorController_State_Machine_Event(&mcState, mcState.queuedEventData, mcState.queuedEventCode);
        return true;
    }
    return false;
}

// Check for error status and raise error event if set
static void QueryError()
{
    if (mcState.error)
    {
        mcState.error = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, ErrorEncountered);
    }
}

// Check reset flag and reset/reinitialize state and I/0 if set
// A reset clears any commands currently in the command buffer
static void QueryReset()
{
    if (mcState.reset)
    {
        // Reinitialize motor controller state, preserving the current state machine state
        MotorController_state_t currentState = mcState.sm_state;
        mcState = MotorController_t();
        mcState.sm_state = currentState;
        
        MotorController::Initialize(&mcState);

        commandBuffer = CommandBuffer();

        MotorController_State_Machine_Reset_EventQueue();
    }
}

// Check if the command buffer cannot receive more data and raise error event if so
static void QueryCommandBufferFull()
{
    if (commandBuffer.IsFull())
    {
        mcState.status = MC_STATUS_COMMAND_BUFFER_FULL;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, ErrorEncountered);
    }
}

// Check if deceleration has started and raise event if so
static void QueryDecelerationStarted()
{
    if (mcState.decelerationStarted)
    {
        mcState.decelerationStarted = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, DecelerationStarted);
    }
}

// Check axis at limit flag and raise event if set
static void QueryAxisAtLimit()
{
    if (mcState.axisAtLimit)
    {
        mcState.axisAtLimit = false;
        EventData eventData;
        MotorController_State_Machine_Event(&mcState, eventData, AxisAtLimit);
    }
}

int main()
{
#ifdef DEBUG
    // Turn on LED
    DDRB |= (1<<DDB0);
    PORTB |= (1<<PB0);
#endif

    // Disable interrupts
    cli();

#ifdef DEBUG
    Debug::Initialize();
#endif

    // Initialize I2C interface
    I2CInterface::Initialize(&mcState);

    // Initialize I/O and subsystems
    MotorController::Initialize(&mcState);

    // Enable interrupts
    sei();

#ifdef DEBUG
    printf_P(PSTR("INFO: Motor controller firmware initialized\n"));
#endif

    // Initialize the state machine to the Disabled state
    MotorController_State_Machine_Init(&mcState, Disabled);

    for(;;)
    {
        QueryReset();
        QueryError();
        QueryCommandBufferFull();
        QueryLimitSwitchInterrupt();
        Planner::PlanHoldCallback();
        QueryMotionComplete();
        QueryAxisAtLimit();
        QueryDecelerationStarted();

         // If QueryEventQueue() returns true, at least one queued event exists
         // If the queue contains one queued event, it may contain additional events
         // Skip querying the command buffer until the event queue empties to ensure commands are handled
         // in the order the controller receives them

        if(QueryEventQueue()) continue;
        QueryCommandBuffer();
    }
}

ISR (LIMIT_SW_ISR_vect)
{
    limitSwitchHit = true;

    // Disable interrupts immediately to avoid spurious interrupts
    LIMIT_SW_PCMSK &= ~Z_AXIS_LIMIT_SW_PCINT_BM;
    LIMIT_SW_PCMSK &= ~R_AXIS_LIMIT_SW_PCINT_BM;
}
