//  File: MotorControllerState.h
//  Type representing instantiation of state machine
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

#ifndef MOTORCONTROLLERSTATE_H
#define MOTORCONTROLLERSTATE_H

#include "AxisSettings.h"
#include "EventData.h"
#include "StateMachine.h"
#include "Status.h"

// All writable data referred to by state machine
// related code goes in this structure
struct MotorControllerState
{
    MotorController_state_t sm_state;           // Current state machine state
    AxisSettings zAxisSettings;                 // Z axis Settings
    AxisSettings rAxisSettings;                 // R axis settings
    bool volatile motionComplete;               // Has the current motion completed? (set in load or DDA timer ISRs)
    bool volatile decelerationStarted;          // Has deceleration started for the current move? (not set for pause deceleration, set in exec timer ISR)
    bool queuedEvent;                           // Has the state machine dequeued an event into queuedEventData?
    bool volatile error;                        // Raise an error encountered event immediately? (possibly set in exec timer ISR)
    bool reset;                                 // Reset the controller immediately?
    bool axisAtLimit;                           // Raise an axis at limit event immediately?
    Status volatile status = MC_STATUS_SUCCESS; // Status code (possibly set in exec timer ISR)
    EventData queuedEventData;                  // EventData for the next queued event to handle
    SM_EVENT_CODE_TYPE queuedEventCode;         // The state machine code of the next queued event to handle
};

#endif  // MOTORCONTROLLERSTATE_H
