/*
 * MotorControllerState.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef MOTORCONTROLLERSTATE_H
#define MOTORCONTROLLERSTATE_H

#include "AxisSettings.h"
#include "EventData.h"
#include "StateMachine.h"
#include "Status.h"

/*
 * Type representing instantiation of state machine
 * All writable data referred to by state machine
 * related code goes in this structure
 */

struct MotorControllerState
{
    MotorController_state_t sm_state;   // Current state machine state
    AxisSettings zAxisSettings;         // Z axis Settings
    AxisSettings rAxisSettings;         // R axis settings
    uint8_t volatile motionComplete;    // Has the current motion completed?
    uint8_t queuedEvent;                // Has the state machine dequeued an event into queuedEventData?
    uint8_t resumeRequested;            // Has the state machine received a resume event when it cannot be handled immediately?
    uint8_t resume;                     // Raise a resume event immediately?
    uint8_t error;                      // Raise an error encountered event immediately?
    Status status;                      // Status code
    EventData queuedEventData;          // EventData for the next queued event to handle
    SM_EVENT_CODE_TYPE queuedEventCode; // The state machine code of the next queued event to handle
};

#endif /* MOTORCONTROLLERSTATE_H */
