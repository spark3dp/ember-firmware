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
/*
 * Motor controller state machine declarations
 * Used as input to SMG (smg.sourceforge.net)
 */

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




// Error handler
void MotorController_State_Machine_Error(
        MotorController_t* stateMachine,
        EventData eventData,
        uint8_t errorID,
        const char* errorText, ...)
{
#ifdef DEBUG
    printf_P(PSTR("ERROR: Fatal state machine error\n"));
#endif /* DEBUG */
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

/*
 * Reinitialize the event queue
 */

void MotorController_State_Machine_Reset_EventQueue()
{
    eventQueue = EventQueue();
}

/*
 * If the event queue is not empty, dequeue and event and store it in the state instance
 */

static void DequeueEvent(MotorController_t* mcState)
{
    if (!eventQueue.IsEmpty())
    {
        mcState->queuedEvent = true;
        eventQueue.Remove(mcState->queuedEventCode, mcState->queuedEventData);
    }
}

/*
 * Check resume requested flag when entering paused state
 * Setting the resume flag results in the main loop raising a resume event only
 * after the state machine enters the paused state
 */

static void QueryResumeRequestedFlag(MotorController_t* mcState)
{
    if (mcState->resumeRequested)
    {
        mcState->resumeRequested = false;
        mcState->resume = true;
    }
}




























/*
---------------------------------------------------------------------------
## State Machine | MotorController
##

##
##     OBJ Type  | MotorController_t*
##     EVT Type  | EventData
##   Num States  | 15
##   Num Events  | 16
##    Num Trans  | 88
## Num Codesegs  | 19
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
      case HomingRAxisDeceleratingForPause:     /* The r axis is
                                                   decelerating to a
                                                   stop from homing
                                                   for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              /**> SetResumeRequestedFlag */

               _sm_obj->resumeRequested = true;
               }
               break;
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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

                              /**> QueryResumeRequestedFlag */

               QueryResumeRequestedFlag(_sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case MovingAxis:     /* An axis is in motion */
       switch (event_code) {
           case EnableRequested:
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
               MovingAxisDeceleratingForPause;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case WaitingForInterruptRequest:     /* The controller is
                                              waiting for an
                                              interrupt request
                                              before handling
                                              enqueued action command
                                              */
       switch (event_code) {
           case EnableRequested:
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
               }
               break;
       }
       break;

      case HomingRAxisPaused:     /* r axis homing is paused */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = HomingRAxis;

                              /**> EndMotionHold */

               MotorController::EndMotionHold();
               }
               break;
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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
               }
               break;
       }
       break;

      case MovingAxisDeceleratingForPause:     /* The currently
                                                  moving axis is
                                                  decelerating to a
                                                  stop for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              /**> SetResumeRequestedFlag */

               _sm_obj->resumeRequested = true;
               }
               break;
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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

                              /**> QueryResumeRequestedFlag */

               QueryResumeRequestedFlag(_sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case HomingZAxis:     /* The z axis is searching for its limit
                               */
       switch (event_code) {
           case EnableRequested:
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
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               HomingZAxisDeceleratingForPause;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case MovingAxisPaused:     /* Axis movement is paused */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> EndMotionHold */

               MotorController::EndMotionHold();
               }
               break;
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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
               }
               break;
       }
       break;

      case ReadyForAction:     /* The controller is ready to handle
                                  an action command (home, move) */
       switch (event_code) {
           case EnableRequested:
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

               MotorController::HomeZAxis(_sm_evt.parameter, _sm_obj);
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

               MotorController::HomeRAxis(_sm_evt.parameter, _sm_obj);
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
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings);

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

               MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
       }
       break;

      case Disabled:     /* The motor drivers and controller are
                            disabled */
       switch (event_code) {
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
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings);
               }
               break;
           case SetRAxisSettingRequested:
               {

                              /**> SetRAxisSetting */

               MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings);
               }
               break;
       }
       break;

      case HomingZAxisDeceleratingForPause:     /* The z axis is
                                                   decelerating to a
                                                   stop from homing
                                                   for pause */
       switch (event_code) {
           case ResumeRequested:
               {

                              /**> SetResumeRequestedFlag */

               _sm_obj->resumeRequested = true;
               }
               break;
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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

                              /**> QueryResumeRequestedFlag */

               QueryResumeRequestedFlag(_sm_obj);
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case HomingRAxis:     /* The r axis is searching for its limit
                               */
       switch (event_code) {
           case EnableRequested:
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
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state =
               HomingRAxisDeceleratingForPause;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

      case HomingZAxisPaused:     /* z axis homing is paused */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = HomingZAxis;

                              /**> EndMotionHold */

               MotorController::EndMotionHold();
               }
               break;
           case MotionComplete:
           case AxisLimitReached:
           case PauseRequested:
           case EnableRequested:
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
               }
               break;
       }
       break;

      case Error:     /* An error has occurred */
       switch (event_code) {
           case EnableRequested:
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
               }
               break;
       }
       break;

      case Ready:     /* The motor drivers are enabled and controller
                         ready to execute any command */
       switch (event_code) {
           case EnableRequested:
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
               }
               break;
           case SetZAxisSettingRequested:
               {

                              /**> SetZAxisSetting */

               MotorController::UpdateSettings(Z_AXIS, _sm_evt,
               _sm_obj->zAxisSettings);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case SetRAxisSettingRequested:
               {

                              /**> SetRAxisSetting */

               MotorController::UpdateSettings(R_AXIS, _sm_evt,
               _sm_obj->rAxisSettings);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
       }
       break;

      case EndingMotion:     /* The axis in motion is decelerating,
                                system will clear planning buffer */
       switch (event_code) {
           case EnableRequested:
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

                              /**> EndMotion */

               MotorController::EndMotion();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case ErrorEncountered:
               {

                              _sm_obj->sm_state = Error;
               }
               break;
       }
       break;

        default:
MotorController_State_Machine_Error(_sm_obj, _sm_evt, 2, "");
    }
}


