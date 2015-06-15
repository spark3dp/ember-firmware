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

/************************************************************************
 **
 ** Include file definitions
 **
----------------------------------------------------------------------
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
----------------------------------------------------------------------

 **/

#ifndef _MotorController_SM_DEFS_
#define _MotorController_SM_DEFS_


typedef uint8_t MotorController_state_t;  /* State Type */
#define UNDEFINED_TRANSITION_RESULT 1

#define HomingZAxis        2    /* The z axis searches for its limit
                                   */
#define Error      3    /* An error occurred */
#define HomingZAxisPaused          4    /* The controller paused z
                                           axis homing */
#define HomingRAxis        5    /* The r axis searches for its limit
                                   */
#define DeceleratingForCompletion          6    /* The axis in motion
                                                   decelerates as a
                                                   result of reaching
                                                   its limit or
                                                   traversing the
                                                   requested distance
                                                   */
#define HomingRAxisDeceleratingForResume           7    /* The r axis
                                                           decelerates
                                                           to a stop
                                                           from
                                                           homing to
                                                           resume
                                                           immediately
                                                           from the
                                                           pause when
                                                           deceleration
                                                           completes
                                                           */
#define MovingAxis         8    /* An axis moves */
#define HomingRAxisPaused          9    /* The controller paused r
                                           axis homing */
#define MovingAxisDeceleratingForResume           10    /* The
                                                           currently
                                                           moving
                                                           axis
                                                           decelerates
                                                           to a stop
                                                           to resume
                                                           immediately
                                                           from the
                                                           pause when
                                                           deceleration
                                                           completes
                                                           */
#define MovingAxisPaused          11    /* The controller paused axis
                                           movement */
#define HomingRAxisDeceleratingForPause           12    /* The r axis
                                                           decelerates
                                                           to a stop
                                                           from
                                                           homing for
                                                           pause */
#define ReadyForAction            13    /* The controller dequeues
                                           and handles the next
                                           action command (home,
                                           move) */
#define Ready     14    /* The controller enabled the motor drivers
                           and handles incoming or queued commands */
#define DeceleratingForSequencePause      15    /* The controller
                                                   received a pause
                                                   command while an
                                                   axis decelerates
                                                   for completion of
                                                   its move (pauses
                                                   handling of the
                                                   queue rather than
                                                   motion) */
#define WaitingForInterruptRequest        16    /* The controller
                                                   waits for an
                                                   interrupt request
                                                   before handling
                                                   queued action
                                                   command */
#define HomingZAxisDeceleratingForResume          17    /* The z axis
                                                           decelerates
                                                           to a stop
                                                           from
                                                           homing to
                                                           resume
                                                           immediately
                                                           from the
                                                           pause when
                                                           deceleration
                                                           completes
                                                           */
#define DeceleratingAfterClear            18    /* The axis in motion
                                                   decelerates as a
                                                   result of a pause
                                                   and but the
                                                   controller
                                                   received a clear
                                                   command before the
                                                   axis stopped
                                                   completely */
#define Disabled          19    /* The controller disabled the motor
                                   drivers and waits for an enable
                                   command */
#define MovingAxisDeceleratingForPause            20    /* The
                                                           currently
                                                           moving
                                                           axis
                                                           decelerates
                                                           to a stop
                                                           for pause
                                                           */
#define HomingZAxisDeceleratingForPause           21    /* The z axis
                                                           decelerates
                                                           to a stop
                                                           from
                                                           homing for
                                                           pause */
#define DeceleratingForSequenceResume     22    /* The controller
                                                   received a pause
                                                   command while an
                                                   axis decelerates
                                                   for completion of
                                                   its move (resumes
                                                   handling of the
                                                   queue when
                                                   deceleration
                                                   completes) */
#define SequencePaused            23    /* The controller has paused
                                           the process of handling
                                           the sequence of commands
                                           in the queue and waits for
                                           a resume command */






typedef uint8_t MotorController_event_t;  /* Event Type */

#define ResetRequested             2    /* Reset command received */
#define HomeZAxisRequested         3    /* Home z axis command
                                           received */
#define HomeRAxisRequested         4    /* Home r axis command
                                           received */
#define MoveZAxisRequested         5    /* Move z axis command
                                           received */
#define MoveRAxisRequested         6    /* Move r axis command
                                           received */
#define DisableRequested           7    /* Disable motor drivers
                                           command received */
#define EnableRequested            8    /* Enable motor drivers
                                           command received */
#define SetZAxisSettingRequested           9    /* Set z axis setting
                                                   command received */
#define SetRAxisSettingRequested          10    /* Set r axis setting
                                                   command received */
#define InterruptRequested        11    /* Generate interrupt command
                                           received */
#define AxisLimitReached          12    /* Axis limit switched
                                           reached */
#define MotionComplete            13    /* All moves in motion
                                           planning buffer have been
                                           executed */
#define PauseRequested            14    /* Pause the current motion
                                           in progress received */
#define ResumeRequested           15    /* Resume the previously
                                           paused motion */
#define ClearRequested            16    /* Clear command received */
#define ErrorEncountered          17    /* An error was encountered */
#define DecelerationStarted       18    /* The moving axis started
                                           decelerating to complete
                                           its move */
#define AxisAtLimit       19    /* The system has found the axis
                                   requested to home at its limit
                                   without requiring movement */











void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                                    MotorController_state_t initial_state);
                


void MotorController_State_Machine_Error( MotorController_t* _sm_obj,
                                      EventData _sm_evt,
                                      uint8_t err_id,
                                      const char *errtext,
                                      ... );
                                      

void MotorController_State_Machine_Event( MotorController_t* _sm_obj,
                                      EventData _sm_evt,
                                      MotorController_event_t event_code );


#ifndef SM_TRACE
#define SM_TRACE_INIT(Obj, Evt, SM_Name, InitState) \
        printf("** SM %s 0x%x: State %d-%s  INIT\n", \
               #SM_Name, Obj, InitState, SM_Name##_State_Name(InitState));
#define SM_TRACE_EVENT(Obj, Evt, SM_Name, Event) \
        printf("** SM %s 0x%x: State %d=%s -- Event %d=%s\n", \
               #SM_Name, Obj, \
               Obj->sm_state, SM_Name##_State_Name(Obj->sm_state), \
               Event, SM_Name##_Event_Name(Event));
#define SM_TRACE_POST_EVENT(Obj, Evt, SM_Name, Event) \
        printf("** SM %s 0x%x: State %d=%s -- Event %d=%s\n", \
               #SM_Name, Obj, \
               Obj->sm_state, SM_Name##_State_Name(Obj->sm_state), \
               Event, SM_Name##_Event_Name(Event));
#define SM_TRACE_EXP_EV(Obj, Evt, SM_Name, Event) \
        printf("** SM %s 0x%x: State %d=%s ++ Event %d=%s\n", \
               #SM_Name, Obj, Obj->sm_state, \
               SM_Name##_State_Name(Obj->sm_state), \
               Event, SM_Name##_Event_Name(Event));
#endif



#endif   /* _MotorController_SM_DEFS_ */
               
