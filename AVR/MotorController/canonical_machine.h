#ifndef canonical_machine_h
#define canonical_machine_h

#include "tinyg.h"
#include "AxisSettings.h"

typedef struct cmSingleton {    // struct to manage cm globals and cycles
  uint16_t magic_start;     // magic number to test memory integity 
  uint8_t cycle_state;
  uint8_t motion_state;
  uint8_t hold_state;       // feedhold sub-state machine
  uint16_t magic_end;
} cmSingleton_t;

extern cmSingleton_t cm;

enum cmFeedholdState {        // feedhold_state machine
  FEEDHOLD_OFF = 0,       // no feedhold in effect
  FEEDHOLD_SYNC,          // start hold - sync to latest aline segment
  FEEDHOLD_PLAN,          // replan blocks for feedhold
  FEEDHOLD_DECEL,         // decelerate to hold point
  FEEDHOLD_HOLD,          // holding
  FEEDHOLD_END_HOLD       // end hold (transient state to OFF)
};

enum cmCycleState {
  CYCLE_OFF = 0,          // machine is idle
  CYCLE_MACHINING,        // machine in normal machining cycle
};

enum cmMotionState {
  MOTION_STOP = 0,        // motion has stopped
  MOTION_RUN,           // machine is in motion
  MOTION_HOLD           // feedhold in progress
};

void cm_init(void);                       // init canonical machine
void cm_cycle_start(void);
void cm_cycle_end(void);

void cm_straight_feed(uint8_t axisIndex, float distance, const AxisSettings& settings);
void cm_begin_feedhold(void);
void cm_end_feedhold(void);

uint8_t cm_get_motion_state(void);
uint8_t cm_get_hold_state(void);
uint8_t cm_get_cycle_state(void);

#endif
