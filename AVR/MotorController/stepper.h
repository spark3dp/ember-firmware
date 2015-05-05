#ifndef stepper_h
#define stepper_h

#include <avr/io.h>
#include <avr/interrupt.h>

void st_request_exec_move(void);
void st_init(void);       // initialize stepper subsystem
uint8_t st_isbusy(void);    // return TRUE is any axis is running (F=idle)
void st_prep_null(void);
stat_t st_prep_line(float steps[], float microseconds);

// The interrupt priority of the DDA and load timers should be above that of the exec timer
// For the atmega328p, timer 2 has the highest priority, then timer 1, followed by timer 0

// Use 16-bit timer 1 for the DDA
#define TIMER_DDA_CTRLA    TCCR1A            // Control register A
#define TIMER_DDA_CTRLB    TCCR1B            // Control register B
#define TIMER_DDA_IMSK     TIMSK1            // Interrupt mask register
#define TIMER_DDA_PERIOD   OCR1A             // Output compare register A (period of the timer)
#define TIMER_DDA_ISR_vect TIMER1_COMPA_vect // Output compare interrupt service routine vector

// Use 8-bit timer with higher priority for the load software interrupt
#define TIMER_LOAD_CTRLA    TCCR2A            // Control register A
#define TIMER_LOAD_CTRLB    TCCR2B            // Control register B
#define TIMER_LOAD_IMSK     TIMSK2            // Interrupt mask register
#define TIMER_LOAD_PERIOD   OCR2A             // Output compare register A (period of the timer)
#define TIMER_LOAD_ISR_vect TIMER2_COMPA_vect // Output compare interrupt service routine vector

// Use 8-bit timer with lower priority for the exec software interrupt
#define TIMER_EXEC_CTRLA    TCCR0A            // Control register A
#define TIMER_EXEC_CTRLB    TCCR0B            // Control register B
#define TIMER_EXEC_IMSK     TIMSK0            // Interrupt mask register
#define TIMER_EXEC_PERIOD   OCR0A             // Output compare register A (period of the timer)
#define TIMER_EXEC_ISR_vect TIMER0_COMPA_vect // Output compare interrupt service routine vector

#define TIMER_ENABLE  (1<<CS00) // Start timer with clock source set to F_CPU
#define TIMER_DISABLE 0         // Stop timer

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

// Motor IO

// DDR - data direction register
// DD_BM - data direction bit mask
// PORT - IO port register
// BM - bit mask

// Common to both motors

// SLEEP - PC0
#define MOTOR_SLEEP_DDR DDRC
#define MOTOR_SLEEP_DD_BM (1<<DDC0)
#define MOTOR_SLEEP_PORT PORTC
#define MOTOR_SLEEP_BM (1<<PC0)

// ENABLE - PC1
#define MOTOR_ENABLE_DDR DDRC
#define MOTOR_ENABLE_DD_BM (1<<DDC1)
#define MOTOR_ENABLE_PORT PORTC
#define MOTOR_ENABLE_BM (1<<PC1)

// RESET - PC2
#define MOTOR_RESET_DDR DDRC
#define MOTOR_RESET_DD_BM (1<<DDC2)
#define MOTOR_RESET_PORT PORTC
#define MOTOR_RESET_BM (1<<PC2)

// MODE0 - PB4
#define MOTOR_MODE0_DDR DDRB
#define MOTOR_MODE0_DD_BM (1<<DDB4)
#define MOTOR_MODE0_PORT PORTB
#define MOTOR_MODE0_BM (1<<PB4)

// MODE1 - PB3
#define MOTOR_MODE1_DDR DDRB
#define MOTOR_MODE1_DD_BM (1<<DDB3)
#define MOTOR_MODE1_PORT PORTB
#define MOTOR_MODE1_BM (1<<PB3)

// MODE2 - PB5
#define MOTOR_MODE2_DDR DDRB
#define MOTOR_MODE2_DD_BM (1<<DDB5)
#define MOTOR_MODE2_PORT PORTB
#define MOTOR_MODE2_BM (1<<PB5)

// Rotation motor (channel 2)

// DIR - PD7
#define MOTOR_2_DIRECTION_DDR DDRD
#define MOTOR_2_DIRECTION_DD_BM (1<<DDD7)
#define MOTOR_2_DIRECTION_PORT PORTD
#define MOTOR_2_DIRECTION_BM  (1<<PD7)

// STEP - PD6
#define MOTOR_2_STEP_DDR DDRD
#define MOTOR_2_STEP_DD_BM (1<<DDD6)
#define MOTOR_2_STEP_PORT PORTD
#define MOTOR_2_STEP_BM (1<<PD6)

// Z axis motor (channel 1)

// Actual motor board in printer uses PD1
// DIR - PD1
//#define MOTOR_1_DIRECTION_DDR DDRD
//#define MOTOR_1_DIRECTION_DD_BM (1<<DDD1)
//#define MOTOR_1_DIRECTION_PORT PORTD
//#define MOTOR_1_DIRECTION_BM  (1<<PD1)

// On the development system, use PD3 so PD1 can be used for UART
// DIR - PD3
#define MOTOR_1_DIRECTION_DDR DDRD
#define MOTOR_1_DIRECTION_DD_BM (1<<DDD3)
#define MOTOR_1_DIRECTION_PORT PORTD
#define MOTOR_1_DIRECTION_BM  (1<<PD3)

// STEP - PB2
#define MOTOR_1_STEP_DDR DDRB
#define MOTOR_1_STEP_DD_BM (1<<DDB2)
#define MOTOR_1_STEP_PORT PORTB
#define MOTOR_1_STEP_BM (1<<PB2)

#endif
