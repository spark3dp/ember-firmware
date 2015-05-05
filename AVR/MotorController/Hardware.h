/*
 * Hardware.h
 * Author: Jason Lefley
 * Date  : 2015-05-04
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <avr/io.h>

// Motor I/O definitions

// DD_BM - data direction bit mask
// PORT - IO port register
// DDR - data direction register
// BM - bit mask

// Common to both motors

// RESET - PC2
#define MOTOR_RESET_DDR DDRC
#define MOTOR_RESET_DD_BM (1<<DDC2)
#define MOTOR_RESET_PORT PORTC
#define MOTOR_RESET_BM (1<<PC2)

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

// R axis motor

// DIR - PD7
#define MOTOR_R_DIRECTION_DDR DDRD
#define MOTOR_R_DIRECTION_DD_BM (1<<DDD7)
#define MOTOR_R_DIRECTION_PORT PORTD
#define MOTOR_R_DIRECTION_BM  (1<<PD7)

// STEP - PD6
#define MOTOR_R_STEP_DDR DDRD
#define MOTOR_R_STEP_DD_BM (1<<DDD6)
#define MOTOR_R_STEP_PORT PORTD
#define MOTOR_R_STEP_BM (1<<PD6)

// Z axis motor

// Actual motor board in printer uses PD1
// DIR - PD1
//#define MOTOR_Z_DIRECTION_DDR DDRD
//#define MOTOR_Z_DIRECTION_DD_BM (1<<DDD1)
//#define MOTOR_Z_DIRECTION_PORT PORTD
//#define MOTOR_Z_DIRECTION_BM  (1<<PD1)

// On the development system, use PD3 so PD1 can be used for UART
// DIR - PD3
#define MOTOR_Z_DIRECTION_DDR DDRD
#define MOTOR_Z_DIRECTION_DD_BM (1<<DDD3)
#define MOTOR_Z_DIRECTION_PORT PORTD
#define MOTOR_Z_DIRECTION_BM  (1<<PD3)

// STEP - PB2
#define MOTOR_Z_STEP_DDR DDRB
#define MOTOR_Z_STEP_DD_BM (1<<DDB2)
#define MOTOR_Z_STEP_PORT PORTB
#define MOTOR_Z_STEP_BM (1<<PB2)

// Limit switches
#define Z_AXIS_LIMIT_SW_DDR DDRD
#define Z_AXIS_LIMIT_SW_DD_BM (1<<DDD4)
#define Z_AXIS_LIMIT_SW_PORT PORTD
#define Z_AXIS_LIMIT_SW_BM (1<<PD4)
#define Z_AXIS_LIMIT_SW_HIT !(PIND & Z_AXIS_LIMIT_SW_BM)
#define Z_AXIS_LIMIT_SW_PCINT_BM (1<<PCINT20)

#define R_AXIS_LIMIT_SW_DDR DDRD
#define R_AXIS_LIMIT_SW_DD_BM (1<<DDD2)
#define R_AXIS_LIMIT_SW_PORT PORTD
#define R_AXIS_LIMIT_SW_BM (1<<PD2)
#define R_AXIS_LIMIT_SW_HIT !(PIND & R_AXIS_LIMIT_SW_BM)
#define R_AXIS_LIMIT_SW_PCINT_BM (1<<PCINT18)

#define LIMIT_SW_PCMSK PCMSK2
#define LIMIT_SW_PCIE_BM (1<<PCIE2)

#endif /* HARDWARE_H */
