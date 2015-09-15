/*

                    ####                             ########### 
                   ######                   ####################### 
                  ########      ######################################
                  ########      ######################################
                   #######            ###############################
                    ####                             ########### 

    WARNING: This file has been automatically generated.  Any editing
             performed directly on this file will be lost if the file
             is regenerated.

             SMG v1.7.4

*/
//  File: StateMachine.sm
//  Motor controller state machine declarations
//  Used as input to SMG (smg.sourceforge.net)
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

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

// Trace logging macros
#define SM_TRACE
#define SM_TRACE_INIT(Obj, Evt, SM_Name, InitState) \
        printf_P(PSTR("INFO: State machine (%S 0x%x) initialized, current state: %S\n\n"), \
               PSTR(#SM_Name), Obj, MotorController_State_Name(InitState));
#define SM_TRACE_EVENT(Obj, Evt, SM_Name, Event) \
        printf_P(PSTR("INFO: State machine (%S 0x%x) handling %S in %S\n"), \
               PSTR(#SM_Name), Obj, MotorController_Event_Name(Event), MotorController_State_Name(Obj->sm_state));
#define SM_TRACE_POST_EVENT(Obj, Evt, SM_Name, Event) \
        printf_P(PSTR("INFO: State machine (%S 0x%x) transaction complete, current state: %S\n\n"), \
               PSTR(#SM_Name), Obj, MotorController_State_Name(Obj->sm_state));
#define SM_TRACE_EXP_EV(Obj, Evt, SM_Name, Event) \
        printf(PSTR("** SM %S 0x%x: State %d ++ Event %d\n"), \
               PSTR(#SM_Name), Obj, Obj->sm_state, Event);

#include "StateMachine.h"
#include "MotorController.h"
#include "Motors.h"
#include "EventData.h"
#include "EventQueue.h"
#include "MachineDefinitions.h"
#include "Planner.h"




// State machine error handler
// Called if invalid transition encountered
void MotorController_State_Machine_Error(
        MotorController_t* mcState,
        EventData eventData,
        uint8_t errorID,
        const char* errorText, ...)
{
#ifdef DEBUG
    printf_P(PSTR("ERROR: Fatal state machine error\n"));
#endif

    mcState->status = MC_STATUS_STATE_MACHINE_ERROR;
    mcState->error = true;
}

#define CHECK_STATUS(function, mcState)  \
    do                                   \
    {                                    \
        Status status = function;        \
        if (status != MC_STATUS_SUCCESS) \
        {                                \
            mcState->status = status;    \
            mcState->error = true;       \
        }                                \
    }                                    \
    while (0)

static EventQueue eventQueue;

// Reinitialize the event queue
void MotorController_State_Machine_Reset_EventQueue()
{
    eventQueue = EventQueue();
}

// If the event queue is not empty, dequeue an event and store it in the state instance
static void DequeueEvent(MotorController_t* mcState)
{
    if (!eventQueue.IsEmpty())
    {
        mcState->queuedEvent = true;
        eventQueue.Remove(mcState->queuedEventCode, mcState->queuedEventData);
    }
}




























/*
---------------------------------------------------------------------------
## State Machine | MotorController
##

##
##     OBJ Type  | MotorController_t*
##     EVT Type  | EventData
##   Num States  | 22
##   Num Events  | 18
##    Num Trans  | 101
## Num Codesegs  | 17
##   Definition  | Evaluated Good Complete
---------------------------------------------------------------------------
 */


void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                           MotorController_state_t initial_state)
{
    _sm_obj->sm_state = initial_state;
}


void
MotorController_State_Machine_Event(
    MotorController_t* _sm_obj,
    EventData _sm_evt,
    MotorController_event_t event_code
    )
{
    switch (_sm_obj->sm_state) {
      case HomingZAxis:     /* The z axis searches for its limit */
       switch (event_code) {
           case EnableRequested:
           case MotionComplete:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case DecelerationStarted:
               {

                              _sm_obj->sm_state = DeceleratingForCompletion;
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case AxisAtLimit:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = DeceleratingForCompletion;

                              /**> BeginHold */

               Planner::BeginHold();
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               HomingZAxisDeceleratingForPause;

                              /**> BeginHold */

               Planner::BeginHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case Error:     /* An error occurred */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingZAxisPaused:     /* The controller paused z axis
                                     homing */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = HomingZAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingRAxis:     /* The r axis searches for its limit */
       switch (event_code) {
           case EnableRequested:
           case MotionComplete:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case DecelerationStarted:
               {

                              _sm_obj->sm_state = DeceleratingForCompletion;
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case AxisAtLimit:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = DeceleratingForCompletion;

                              /**> BeginHold */

               Planner::BeginHold();
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               HomingRAxisDeceleratingForPause;

                              /**> BeginHold */

               Planner::BeginHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case DeceleratingForCompletion:     /* The axis in motion
                                             decelerates as a result
                                             of reaching its limit or
                                             traversing the requested
                                             distance */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               DeceleratingForSequencePause;
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingRAxisDeceleratingForResume:     /* The r axis
                                                    decelerates to a
                                                    stop from homing
                                                    to resume
                                                    immediately from
                                                    the pause when
                                                    deceleration
                                                    completes */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = HomingRAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case MovingAxis:     /* An axis moves */
       switch (event_code) {
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case DecelerationStarted:
               {

                              _sm_obj->sm_state = DeceleratingForCompletion;
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               MovingAxisDeceleratingForPause;

                              /**> BeginHold */

               Planner::BeginHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingRAxisPaused:     /* The controller paused r axis
                                     homing */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = HomingRAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case MovingAxisDeceleratingForResume:     /* The currently
                                                   moving axis
                                                   decelerates to a
                                                   stop to resume
                                                   immediately from
                                                   the pause when
                                                   deceleration
                                                   completes */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case MovingAxisPaused:     /* The controller paused axis
                                    movement */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingRAxisDeceleratingForPause:     /* The r axis
                                                   decelerates to a
                                                   stop from homing
                                                   for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state =
               HomingRAxisDeceleratingForResume;
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = HomingRAxisPaused;
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = DeceleratingAfterClear;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case ReadyForAction:     /* The controller dequeues and handles
                                  the next action command (home,
                                  move) */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case HomeZAxisRequested:
               {

                              _sm_obj->sm_state = HomingZAxis;

                              /**> HomeZAxis */

               CHECK_STATUS(MotorController::HomeZAxis(_sm_evt.parameter,
               _sm_obj), _sm_obj);
               }
               break;
           case InterruptRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> GenerateInterrupt */

               MotorController::GenerateInterrupt();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case HomeRAxisRequested:
               {

                              _sm_obj->sm_state = HomingRAxis;

                              /**> HomeRAxis */

               CHECK_STATUS(MotorController::HomeRAxis(_sm_evt.parameter,
               _sm_obj), _sm_obj);
               }
               break;
           case DisableRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MoveRAxisRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> MoveRAxis */

               CHECK_STATUS(MotorController::Move(R_AXIS, _sm_evt.parameter,
               _sm_obj->rAxisSettings), _sm_obj);
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings), _sm_obj);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case MoveZAxisRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> MoveZAxis */

               CHECK_STATUS(MotorController::Move(Z_AXIS, _sm_evt.parameter,
               _sm_obj->zAxisSettings), _sm_obj);
               }
               break;
           case SetRAxisSettingRequested:
               {

                              /**> SetRAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings), _sm_obj);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
       }
       break;

      case Ready:     /* The controller enabled the motor drivers and
                         handles incoming or queued commands */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case HomeRAxisRequested:
           case MoveRAxisRequested:
           case MoveZAxisRequested:
           case HomeZAxisRequested:
               {

                              _sm_obj->sm_state = WaitingForInterruptRequest;

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case InterruptRequested:
               {

                              /**> GenerateInterrupt */

               MotorController::GenerateInterrupt();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case DisableRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings), _sm_obj);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case SetRAxisSettingRequested:
               {

                              /**> SetRAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings), _sm_obj);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
       }
       break;

      case DeceleratingForSequencePause:     /* The controller
                                                received a pause
                                                command while an axis
                                                decelerates for
                                                completion of its
                                                move (pauses handling
                                                of the queue rather
                                                than motion) */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state =
               DeceleratingForSequenceResume;
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = SequencePaused;

                              /**> EndMotion */

               MotorController::EndMotion();
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = DeceleratingAfterClear;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case WaitingForInterruptRequest:     /* The controller waits
                                              for an interrupt
                                              request before handling
                                              queued action command */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case InterruptRequested:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingZAxisDeceleratingForResume:     /* The z axis
                                                    decelerates to a
                                                    stop from homing
                                                    to resume
                                                    immediately from
                                                    the pause when
                                                    deceleration
                                                    completes */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = HomingZAxis;

                              /**> EndHold */

               Planner::EndHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case DeceleratingAfterClear:     /* The axis in motion
                                          decelerates as a result of
                                          a pause and but the
                                          controller received a clear
                                          command before the axis
                                          stopped completely */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = Ready;

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case Disabled:     /* The controller disabled the motor drivers
                            and waits for an enable command */
       switch (event_code) {
           case DecelerationStarted:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case EnableRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> EnableMotorDrivers */

               Motors::Enable();
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case MoveZAxisRequested:
           case HomeZAxisRequested:
               {
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings), _sm_obj);
               }
               break;
           case SetRAxisSettingRequested:
               {

                              /**> SetRAxisSetting */

               CHECK_STATUS(MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings), _sm_obj);
               }
               break;
       }
       break;

      case MovingAxisDeceleratingForPause:     /* The currently
                                                  moving axis
                                                  decelerates to a
                                                  stop for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state =
               MovingAxisDeceleratingForResume;
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = MovingAxisPaused;
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = DeceleratingAfterClear;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case HomingZAxisDeceleratingForPause:     /* The z axis
                                                   decelerates to a
                                                   stop from homing
                                                   for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state =
               HomingZAxisDeceleratingForResume;
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = HomingZAxisPaused;
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = DeceleratingAfterClear;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case DeceleratingForSequenceResume:     /* The controller
                                                 received a pause
                                                 command while an
                                                 axis decelerates for
                                                 completion of its
                                                 move (resumes
                                                 handling of the
                                                 queue when
                                                 deceleration
                                                 completes) */
       switch (event_code) {
           case DecelerationStarted:
           case EnableRequested:
           case AxisAtLimit:
           case AxisLimitReached:
           case ClearRequested:
           case PauseRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

      case SequencePaused:     /* The controller has paused the
                                  process of handling the sequence of
                                  commands in the queue and waits for
                                  a resume command */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = ReadyForAction;

                              /**> DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case EnableRequested:
           case AxisAtLimit:
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case DecelerationStarted:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Disabled;

                              /**> SetResetFlag */

               _sm_obj->reset = true;
               }
               break;
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case HomeZAxisRequested:
               {

                              /**> EnqueueEvent */

               CHECK_STATUS(eventQueue.Add(event_code, _sm_evt), _sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;

                              /**> DisableMotorDrivers */

               Motors::Disable();
               }
               break;
       }
       break;

        default:
MotorController_State_Machine_Error(_sm_obj, _sm_evt, 2, "");
    }
}


