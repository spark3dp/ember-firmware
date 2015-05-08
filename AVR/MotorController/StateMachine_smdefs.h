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
##     EVT Type  | Command*
##   Num States  | 5
##   Num Events  | 11
##    Num Trans  | 20
## Num Codesegs  | 14
##   Definition  | Evaluated Good Complete
----------------------------------------------------------------------

 **/

#ifndef _MotorController_SM_DEFS_
#define _MotorController_SM_DEFS_


typedef uint8_t MotorController_state_t;  /* State Type */
#define UNDEFINED_TRANSITION_RESULT 1

#define Ready      2    /* The system is in an idle state ready to
                           execute any command */
#define HomingZAxis        3    /* The z axis is searching for its
                                   limit */
#define HomingDeceleration  	   4   
#define HomingRAxis        5    /* The r axis is searching for its
                                   limit */
#define Error      6    /* An error has occured */






typedef uint8_t MotorController_event_t;  /* Event Type */

#define ResetRequested             2    /* Reset command received */
#define HomeZAxisRequested         3    /* Home z axis command
                                           received */
#define HomeRAxisRequested         4    /* Home r axis command
                                           received */
#define EnableZAxisMotorRequested          5    /* Enable z axis
                                                   motor command
                                                   received */
#define EnableRAxisMotorRequested          6    /* Enable r axis
                                                   motor command
                                                   received */
#define DisableZAxisMotorRequested         7    /* Disable z axis
                                                   motor command
                                                   received */
#define DisableRAxisMotorRequested         8    /* Disable r axis
                                                   motor command
                                                   received */
#define SetZAxisSettingRequested           9    /* Set z axis setting
                                                   command received */
#define SetRAxisSettingRequested          10    /* Set r axis setting
                                                   command received */
#define AxisLimitReached          11    /* Axis limit switched
                                           reached */
#define MotionComplete            12    /* All moves in motion
                                           planning buffer have been
                                           executed */











void MotorController_State_Machine_Init(MotorController_t* _sm_obj,
                                    MotorController_state_t initial_state);
                


void MotorController_State_Machine_Error( MotorController_t* _sm_obj,
                                      Command* _sm_evt,
                                      uint8_t err_id,
                                      const char *errtext,
                                      ... );
                                      

void MotorController_State_Machine_Event( MotorController_t* _sm_obj,
                                      Command* _sm_evt,
                                      MotorController_event_t event_code );


#ifndef SM_TRACE
#define SM_TRACE_INIT(Obj, Evt, SM_Name, InitState) \
        printf("** SM %s 0x%x: State %d-%s  INIT\n", \
               #SM_Name, Obj, InitState, SM_Name##_State_Name(InitState));
#define SM_TRACE_EVENT(Obj, Evt, SM_Name, Event, OldState) \
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
               
