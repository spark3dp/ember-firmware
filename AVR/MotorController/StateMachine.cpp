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
        printf_P(PSTR("INFO: State machine (%S 0x%x) initialized, current state %d\n"), \
               PSTR(#SM_Name), Obj, InitState);
#define SM_TRACE_EVENT(Obj, Evt, SM_Name, Event, OldState) \
        printf_P(PSTR("INFO: State machine (%S 0x%x) handled event %d in state %d, now in state %d\n"), \
               PSTR(#SM_Name), Obj, Event, OldState, Obj->sm_state);
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
##   Num States  | 6
##   Num Events  | 14
##    Num Trans  | 53
## Num Codesegs  | 17
##   Definition  | Evaluated Good Complete
---------------------------------------------------------------------------
 */


void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                           MotorController_state_t initial_state)
{
    SM_TRACE_INIT(_sm_obj, NULL, MotorController, initial_state);
    _sm_obj->sm_state = initial_state;
}


void
MotorController_State_Machine_Event(
    MotorController_t* _sm_obj,
    EventData _sm_evt,
    MotorController_event_t event_code
    )
{
    uint8_t old_state = _sm_obj->sm_state;

    switch (_sm_obj->sm_state) {
      case MovingAxis:     /* An axis is in motion */
       switch (event_code) {
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
               }
               break;
       }
       break;

      case HomingZAxis:     /* The z axis is searching for its limit
                               */
       switch (event_code) {
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

                              _sm_obj->sm_state = HomingDeceleration;

                              /**> HandleAxisLimitReached */

               MotorController::HandleAxisLimitReached();
               }
               break;
       }
       break;

      case HomingRAxis:     /* The r axis is searching for its limit
                               */
       switch (event_code) {
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

                              _sm_obj->sm_state = HomingDeceleration;

                              /**> HandleAxisLimitReached */

               MotorController::HandleAxisLimitReached();
               }
               break;
       }
       break;

      case Error:     /* An error has occured */
       switch (event_code) {
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
           case AxisLimitReached:
           case MotionComplete:
               {
               }
               break;
       }
       break;

      case Ready:     /* The system is in an idle state ready to
                         execute any command */
       switch (event_code) {
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
           case AxisLimitReached:
           case MotionComplete:
               {
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

               MotorController::HandleSettingsCommand(_sm_evt,
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

               MotorController::HandleSettingsCommand(_sm_evt,
               _sm_obj->rAxisSettings);

                              /**> Group: DequeueEvent */

               DequeueEvent(_sm_obj);
               }
               break;
       }
       break;

      case HomingDeceleration:    
       switch (event_code) {
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
               }
               break;
       }
       break;

        default:
MotorController_State_Machine_Error(_sm_obj, _sm_evt, 2, "");
    }
    SM_TRACE_EVENT(_sm_obj, _sm_evt, MotorController, event_code, old_state);

}


