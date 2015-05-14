#ifndef stepper_h
#define stepper_h

#include "tinyg.h"
#include "StateMachine.h"

void st_init(MotorController_t* mcState);       // initialize stepper subsystem
uint8_t st_isbusy(void);    // return TRUE is any axis is running (F=idle)
stat_t st_prep_line(float steps[], uint8_t directions[], float microseconds);
void st_prep_null(void);

void st_request_exec_move(void);

#define SOFTWARE_INTERRUPT_PERIOD 99 // Cycles (less one) before interrupt is actually generated after software interrupt is called

#define F_DDA     (float)50000  // DDA frequency in hz.

/* DDA substepping
 *  DDA_SUBSTEPS sets the amount of fractional precision for substepping.
 *  Substepping is kind of like microsteps done in software to make
 *  interpolation more accurate.
 *
 *  Set to 1 to disable, but don't do this or you will lose a lot of accuracy.
 */
#define DDA_SUBSTEPS 100000   // 100,000 accumulates substeps to 6 decimal places

/* Accumulator resets
 *  You want to reset the DDA accumulators if the new ticks value is way less 
 *  than previous value, but otherwise you should leave the accumulators alone.
 *  Preserving the accumulator value from the previous segment aligns pulse 
 *  phasing between segments. However, if the new accumulator is going to be 
 *  much less than the old one you must reset it or risk motor stalls.
 */
#define ACCUMULATOR_RESET_FACTOR 2  // amount counter range can safely change

// handy macro
#define _f_to_period(f) (uint16_t)((float)F_CPU / (float)f)

#endif
