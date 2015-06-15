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
    bool volatile motionComplete;       // Has the current motion completed?
    bool volatile decelerationStarted;  // Has deceleration started for the current move? (not set for pause deceleration)
    bool queuedEvent;                   // Has the state machine dequeued an event into queuedEventData?
    bool error;                         // Raise an error encountered event immediately?
    bool reset;                         // Reset the controller immediately?
    bool axisAtLimit;                   // Raise an axis at limit event immediately?
    Status status = MC_STATUS_SUCCESS;  // Status code
    EventData queuedEventData;          // EventData for the next queued event to handle
    SM_EVENT_CODE_TYPE queuedEventCode; // The state machine code of the next queued event to handle
};

#endif /* MOTORCONTROLLERSTATE_H */
