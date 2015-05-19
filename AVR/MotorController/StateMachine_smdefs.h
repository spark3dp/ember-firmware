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
##   Num States  | 9
##   Num Events  | 15
##    Num Trans  | 62
## Num Codesegs  | 19
##   Definition  | Evaluated Good Complete
----------------------------------------------------------------------

 **/

#ifndef _MotorController_SM_DEFS_
#define _MotorController_SM_DEFS_


typedef uint8_t MotorController_state_t;  /* State Type */
#define UNDEFINED_TRANSITION_RESULT 1

#define PausingDeceleration        2    /* The currently pausing axis
                                           is decelerating to a stop
                                           */
#define MovingAxis         3    /* An axis is in motion */
#define HomingZAxis        4    /* The z axis is searching for its
                                   limit */
#define Disabled           5    /* The motor drivers and controller
                                   are disabled */
#define HomingRAxis        6    /* The r axis is searching for its
                                   limit */
#define Error      7    /* An error has occured */
#define Ready      8    /* The motor drivers are enabled and
                           controller ready to execute any command */
#define Paused             9    /* Motion is paused */
#define EndingMotion      10    /* The axis in motion is
                                   decelerating, system will clear
                                   planning buffer */






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











void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                                    MotorController_state_t initial_state);
                

const PROGMEM char* MotorController_State_Name(MotorController_state_t state);  /* Get State Name */
const PROGMEM char* MotorController_State_Desc(MotorController_state_t state);  /* Get State Desc */
const PROGMEM char* MotorController_Event_Name(MotorController_event_t event);  /* Get Event Name */
const PROGMEM char* MotorController_Event_Desc(MotorController_event_t event);  /* Get Event Desc */


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
               
