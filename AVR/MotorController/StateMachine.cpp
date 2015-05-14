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

static EventQueue eventQueue;

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
##   Num States  | 8
##   Num Events  | 17
##    Num Trans  | 63
## Num Codesegs  | 19
##   Definition  | Evaluated Good Complete
---------------------------------------------------------------------------
 */


void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                           MotorController_state_t initial_state)
{
    SM_TRACE_INIT(_sm_obj, NULL, MotorController, initial_state);
    _sm_obj->sm_state = initial_state;
}


const PROGMEM char*
MotorController_State_Name(MotorController_state_t state)
{
    switch (state) {
        case PausingDeceleration: return PSTR("PausingDeceleration");
        case MovingAxis: return PSTR("MovingAxis");
        case HomingZAxis: return PSTR("HomingZAxis");
        case Paused: return PSTR("Paused");
        case HomingRAxis: return PSTR("HomingRAxis");
        case Error: return PSTR("Error");
        case Ready: return PSTR("Ready");
        case EndingMotion: return PSTR("EndingMotion");
    default: return PSTR("??unknown??");
    }
}


const PROGMEM char*
MotorController_State_Desc(MotorController_state_t state)
{
    switch (state) {
        case PausingDeceleration: return PSTR("The currently pausing axis is decelerating to a stop");
        case MovingAxis: return PSTR("An axis is in motion");
        case HomingZAxis: return PSTR("The z axis is searching for its limit");
        case Paused: return PSTR("Motion is paused");
        case HomingRAxis: return PSTR("The r axis is searching for its limit");
        case Error: return PSTR("An error has occured");
        case Ready: return PSTR("The system is in an idle state ready to execute any command");
        case EndingMotion: return PSTR("The axis in motion is decelerating, system will clear planning buffer");
    default: return PSTR("??unknown??");
    }
}


const PROGMEM char*
MotorController_Event_Name(MotorController_event_t event)
{
    switch (event) {
        case ResetRequested: return PSTR("ResetRequested");
        case HomeZAxisRequested: return PSTR("HomeZAxisRequested");
        case HomeRAxisRequested: return PSTR("HomeRAxisRequested");
        case MoveZAxisRequested: return PSTR("MoveZAxisRequested");
        case MoveRAxisRequested: return PSTR("MoveRAxisRequested");
        case EnableZAxisMotorRequested: return PSTR("EnableZAxisMotorRequested");
        case EnableRAxisMotorRequested: return PSTR("EnableRAxisMotorRequested");
        case DisableZAxisMotorRequested: return PSTR("DisableZAxisMotorRequested");
        case DisableRAxisMotorRequested: return PSTR("DisableRAxisMotorRequested");
        case SetZAxisSettingRequested: return PSTR("SetZAxisSettingRequested");
        case SetRAxisSettingRequested: return PSTR("SetRAxisSettingRequested");
        case InterruptRequested: return PSTR("InterruptRequested");
        case AxisLimitReached: return PSTR("AxisLimitReached");
        case MotionComplete: return PSTR("MotionComplete");
        case PauseRequested: return PSTR("PauseRequested");
        case ResumeRequested: return PSTR("ResumeRequested");
        case ClearRequested: return PSTR("ClearRequested");
    default: return PSTR("??unknown??");
    }
}


const PROGMEM char*
MotorController_Event_Desc(MotorController_event_t event)
{
    switch (event) {
        case ResetRequested: return PSTR("Reset command received");
        case HomeZAxisRequested: return PSTR("Home z axis command received");
        case HomeRAxisRequested: return PSTR("Home r axis command received");
        case MoveZAxisRequested: return PSTR("Move z axis command received");
        case MoveRAxisRequested: return PSTR("Move r axis command received");
        case EnableZAxisMotorRequested: return PSTR("Enable z axis motor command received");
        case EnableRAxisMotorRequested: return PSTR("Enable r axis motor command received");
        case DisableZAxisMotorRequested: return PSTR("Disable z axis motor command received");
        case DisableRAxisMotorRequested: return PSTR("Disable r axis motor command received");
        case SetZAxisSettingRequested: return PSTR("Set z axis setting command received");
        case SetRAxisSettingRequested: return PSTR("Set r axis setting command received");
        case InterruptRequested: return PSTR("Generate interrupt command received");
        case AxisLimitReached: return PSTR("Axis limit switched reached");
        case MotionComplete: return PSTR("All moves in motion planning buffer have been executed");
        case PauseRequested: return PSTR("Pause the current motion in progress received");
        case ResumeRequested: return PSTR("Resume the previously paused motion");
        case ClearRequested: return PSTR("Clear command received");
    default: return PSTR("??unknown??");
    }
}


void
MotorController_State_Machine_Event(
    MotorController_t* _sm_obj,
    EventData _sm_evt,
    MotorController_event_t event_code
    )
{
    SM_TRACE_EVENT(_sm_obj, _sm_evt, MotorController, event_code);

    switch (_sm_obj->sm_state) {
      case PausingDeceleration:     /* The currently pausing axis is
                                       decelerating to a stop */
       switch (event_code) {
           case ResumeRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> EndMotionHold */

               MotorController::EndMotionHold();
               }
               break;
           case AxisLimitReached:
           case PauseRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
               }
               break;
           case MotionComplete:
               {

                              _sm_obj->sm_state = Paused;
               }
               break;
           case ClearRequested:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> ClearEventQueue */

               eventQueue.Clear();
               }
               break;
       }
       break;

      case MovingAxis:     /* An axis is in motion */
       switch (event_code) {
           case ClearRequested:
           case AxisLimitReached:
           case ResumeRequested:
               {
               }
               break;
           case PauseRequested:
               {

                              _sm_obj->sm_state = PausingDeceleration;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
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
       }
       break;

      case HomingZAxis:     /* The z axis is searching for its limit
                               */
       switch (event_code) {
           case PauseRequested:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
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
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
       }
       break;

      case Paused:     /* Motion is paused */
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
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
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
       }
       break;

      case HomingRAxis:     /* The r axis is searching for its limit
                               */
       switch (event_code) {
           case PauseRequested:
           case ClearRequested:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
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
           case AxisLimitReached:
               {

                              _sm_obj->sm_state = EndingMotion;

                              /**> BeginMotionHold */

               MotorController::BeginMotionHold();
               }
               break;
       }
       break;

      case Error:     /* An error has occured */
       switch (event_code) {
           case PauseRequested:
           case MotionComplete:
           case ClearRequested:
           case AxisLimitReached:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {
               }
               break;
       }
       break;

      case Ready:     /* The system is in an idle state ready to
                         execute any command */
       switch (event_code) {
           case PauseRequested:
           case MotionComplete:
           case ClearRequested:
           case AxisLimitReached:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case EnableZAxisMotorRequested:
               {

                              /**> EnableZAxisMotor */

               Motors::Enable();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case HomeZAxisRequested:
               {

                              _sm_obj->sm_state = HomingZAxis;

                              /**> HomeZAxis */

               MotorController::HomeZAxis(_sm_evt.parameter, _sm_obj);
               }
               break;
           case EnableRAxisMotorRequested:
               {

                              /**> EnableRAxisMotor */

               Motors::Enable();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
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
           case HomeRAxisRequested:
               {

                              _sm_obj->sm_state = HomingRAxis;

                              /**> HomeRAxis */

               MotorController::HomeRAxis(_sm_evt.parameter, _sm_obj);
               }
               break;
           case DisableZAxisMotorRequested:
               {

                              /**> DisableZAxisMotor */

               Motors::Disable();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case DisableRAxisMotorRequested:
               {

                              /**> DisableRAxisMotor */

               Motors::Disable();

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
           case MoveRAxisRequested:
               {

                              _sm_obj->sm_state = MovingAxis;

                              /**> MoveRAxis */

               MotorController::Move(R_AXIS, _sm_evt.parameter,
               _sm_obj->rAxisSettings);
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

               MotorController::Move(Z_AXIS, _sm_evt.parameter,
               _sm_obj->zAxisSettings);
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
           case PauseRequested:
           case ClearRequested:
           case AxisLimitReached:
           case ResumeRequested:
               {
               }
               break;
           case ResetRequested:
               {

                              _sm_obj->sm_state = Ready;

                              /**> ResetMotorController */

               MotorController::Reset();
               }
               break;
           case HomeZAxisRequested:
           case EnableRAxisMotorRequested:
           case InterruptRequested:
           case HomeRAxisRequested:
           case DisableZAxisMotorRequested:
           case DisableRAxisMotorRequested:
           case MoveRAxisRequested:
           case SetZAxisSettingRequested:
           case MoveZAxisRequested:
           case SetRAxisSettingRequested:
           case EnableZAxisMotorRequested:
               {

                              /**> EnqueueEvent */

               eventQueue.Add(event_code, _sm_evt);
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
       }
       break;

        default:
MotorController_State_Machine_Error(_sm_obj, _sm_evt, 2, "");
    }
    SM_TRACE_POST_EVENT(_sm_obj, _sm_evt, MotorController, event_code);

}


