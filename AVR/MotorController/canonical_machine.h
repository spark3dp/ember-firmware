#ifndef canonical_machine_h
#define canonical_machine_h

#include "tinyg.h"

typedef struct cmSingleton {    // struct to manage cm globals and cycles
  uint16_t magic_start;     // magic number to test memory integity 
  uint8_t combined_state;     // combination of states for display purposes
  uint8_t machine_state;      // machine/cycle/motion is the actual machine state
  uint8_t cycle_state;
  uint8_t motion_state;
  uint8_t hold_state;       // feedhold sub-state machine
  uint8_t feedhold_requested;   // feedhold character has been received
  uint8_t queue_flush_requested;  // queue flush character has been received
  uint8_t cycle_start_requested;  // cycle start character has been received (flag to end feedhold)
  uint8_t homing_state;     // homing cycle sub-state machine
  uint8_t homed[AXES];      // individual axis homing flags
  uint8_t status_report_request;  // 0=no request, 1=timed request, 2=run one now 
  uint32_t status_report_counter; // status report RTC counter for minimum timing
  uint8_t g28_flag;       // true = complete a G28 move
  uint8_t g30_flag;       // true = complete a G30 move
  uint8_t g10_persist_flag;   //.G10 changed offsets - persist them
  uint16_t magic_end;
} cmSingleton_t;

cmSingleton_t cm;

enum cmMotionMode {           // G Modal Group 1
  MOTION_MODE_STRAIGHT_TRAVERSE=0,  // G0 - seek
  MOTION_MODE_STRAIGHT_FEED,      // G1 - feed
};

enum cmFeedholdState {        // feedhold_state machine
  FEEDHOLD_OFF = 0,       // no feedhold in effect
  FEEDHOLD_SYNC,          // start hold - sync to latest aline segment
  FEEDHOLD_PLAN,          // replan blocks for feedhold
  FEEDHOLD_DECEL,         // decelerate to hold point
  FEEDHOLD_HOLD,          // holding
  FEEDHOLD_END_HOLD       // end hold (transient state to OFF)
};

enum cmMachineState {
  MACHINE_INITIALIZING = 0,   // machine is initializing
  MACHINE_READY,          // machine is ready for use
  MACHINE_ALARM,          // machine is in alarm state (shutdown)
  MACHINE_PROGRAM_STOP,     // program stop or no more blocks
  MACHINE_PROGRAM_END,      // program end
  MACHINE_CYCLE,          // machine is running (cycling)
};

enum cmCycleState {
  CYCLE_OFF = 0,          // machine is idle
  CYCLE_MACHINING,        // machine in normal machining cycle
  CYCLE_PROBE,          // machine in probe cycle
  CYCLE_HOMING,         // homing is treated as a specialized cycle
  CYCLE_JOG           // jogging is treated as a specialized cycle
};

enum cmMotionState {
  MOTION_STOP = 0,        // motion has stopped
  MOTION_RUN,           // machine is in motion
  MOTION_HOLD           // feedhold in progress
};

// #### LAYER 8 CRITICAL REGION ###
// #### DO NOT CHANGE THESE ENUMERATIONS WITHOUT COMMUNITY INPUT #### 
enum cmCombinedState {        // check alignment with messages in config.c / msg_stat strings
  COMBINED_INITIALIZING = 0,    // [0] machine is initializing
  COMBINED_READY,         // [1] machine is ready for use
  COMBINED_ALARM,         // [2] machine is in alarm state (shut down)
  COMBINED_PROGRAM_STOP,      // [3] program stop or no more blocks
  COMBINED_PROGRAM_END,     // [4] program end
  COMBINED_RUN,         // [5] motion is running
  COMBINED_HOLD,          // [6] motion is holding
  COMBINED_PROBE,         // [7] probe cycle active
  COMBINED_CYCLE,         // [8] machine is running (cycling)
  COMBINED_HOMING,        // [9] homing is treated as a cycle
  COMBINED_JOG          // [10] jogging is treated as a cycle
};
//#### END CRITICAL REGION ####

void cm_init(void);                       // init canonical machine
void cm_cycle_start(void);
void cm_cycle_end(void);

#endif
