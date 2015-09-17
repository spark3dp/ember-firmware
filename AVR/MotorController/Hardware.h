//  File: Hardware.h
//  Hardware specific definitions
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HARDWARE_H
#define HARDWARE_H

#include <avr/io.h>

// I/O definitions

// DD_BM - data direction bit mask
// PORT - IO port register
// DDR - data direction register
// BM - bit mask

// Motor drivers RESET - PC2
#define MOTOR_RESET_DDR   DDRC
#define MOTOR_RESET_DD_BM (1<<DDC2)
#define MOTOR_RESET_PORT  PORTC
#define MOTOR_RESET_BM    (1<<PC2)

// Motor drivers SLEEP - PC0
#define MOTOR_SLEEP_DDR   DDRC
#define MOTOR_SLEEP_DD_BM (1<<DDC0)
#define MOTOR_SLEEP_PORT  PORTC
#define MOTOR_SLEEP_BM    (1<<PC0)

// Motor drivers ENABLE - PC1
#define MOTOR_ENABLE_DDR   DDRC
#define MOTOR_ENABLE_DD_BM (1<<DDC1)
#define MOTOR_ENABLE_PORT  PORTC
#define MOTOR_ENABLE_BM    (1<<PC1)

// Motor drivers MODE0 - PB4
#define MOTOR_MODE0_DDR   DDRB
#define MOTOR_MODE0_DD_BM (1<<DDB4)
#define MOTOR_MODE0_PORT  PORTB
#define MOTOR_MODE0_BM    (1<<PB4)

// Motor drivers MODE1 - PB3
#define MOTOR_MODE1_DDR   DDRB
#define MOTOR_MODE1_DD_BM (1<<DDB3)
#define MOTOR_MODE1_PORT  PORTB
#define MOTOR_MODE1_BM    (1<<PB3)

// Motor drivers MODE2 - PB5
#define MOTOR_MODE2_DDR   DDRB
#define MOTOR_MODE2_DD_BM (1<<DDB5)
#define MOTOR_MODE2_PORT  PORTB
#define MOTOR_MODE2_BM    (1<<PB5)

// R axis motor DIR - PD7
#define MOTOR_R_DIRECTION_DDR   DDRD
#define MOTOR_R_DIRECTION_DD_BM (1<<DDD7)
#define MOTOR_R_DIRECTION_PORT  PORTD
#define MOTOR_R_DIRECTION_BM    (1<<PD7)

// R axis motor STEP - PD6
#define MOTOR_R_STEP_DDR   DDRD
#define MOTOR_R_STEP_DD_BM (1<<DDD6)
#define MOTOR_R_STEP_PORT  PORTD
#define MOTOR_R_STEP_BM    (1<<PD6)

// Z axis motor DIR
#ifdef DEBUG
// On the development system, use PD3 so PD1 remains available for UART
#define MOTOR_Z_DIRECTION_DDR   DDRD
#define MOTOR_Z_DIRECTION_DD_BM (1<<DDD3)
#define MOTOR_Z_DIRECTION_PORT  PORTD
#define MOTOR_Z_DIRECTION_BM    (1<<PD3)
#else
// Actual hardware uses PD1
#define MOTOR_Z_DIRECTION_DDR   DDRD
#define MOTOR_Z_DIRECTION_DD_BM (1<<DDD1)
#define MOTOR_Z_DIRECTION_PORT  PORTD
#define MOTOR_Z_DIRECTION_BM    (1<<PD1)
#endif  // DEBUG

// Z axis motor STEP - PB2
#define MOTOR_Z_STEP_DDR   DDRB
#define MOTOR_Z_STEP_DD_BM (1<<DDB2)
#define MOTOR_Z_STEP_PORT  PORTB
#define MOTOR_Z_STEP_BM    (1<<PB2)

// Interrupt signal
#ifdef DEBUG
// On the development system use PD5 so PD0 remains available for UART
#define INTERRUPT_DDR   DDRD
#define INTERRUPT_DD_BM (1<<DDB5)
#define INTERRUPT_PORT  PORTD
#define INTERRUPT_BM    (1<<PD5)
#else
// Actual hardware uses PD0
#define INTERRUPT_DDR   DDRD
#define INTERRUPT_DD_BM (1<<DDB0)
#define INTERRUPT_PORT  PORTD
#define INTERRUPT_BM    (1<<PD0)
#endif  // DEBUG/

// Limit switches
#define Z_AXIS_LIMIT_SW_DDR      DDRD
#define Z_AXIS_LIMIT_SW_DD_BM    (1<<DDD4)
#define Z_AXIS_LIMIT_SW_PORT     PORTD
#define Z_AXIS_LIMIT_SW_BM       (1<<PD4)
#define Z_AXIS_LIMIT_SW_HIT      !(PIND & Z_AXIS_LIMIT_SW_BM)
#define Z_AXIS_LIMIT_SW_PCINT_BM (1<<PCINT20)

#define R_AXIS_LIMIT_SW_DDR      DDRD
#define R_AXIS_LIMIT_SW_DD_BM    (1<<DDD2)
#define R_AXIS_LIMIT_SW_PORT     PORTD
#define R_AXIS_LIMIT_SW_BM       (1<<PD2)
#define R_AXIS_LIMIT_SW_HIT      !(PIND & R_AXIS_LIMIT_SW_BM)
#define R_AXIS_LIMIT_SW_PCINT_BM (1<<PCINT18)

#define LIMIT_SW_PCMSK    PCMSK2
#define LIMIT_SW_PCIE_BM  (1<<PCIE2)
#define LIMIT_SW_ISR_vect PCINT2_vect

// The interrupt priority of the DDA and load timers should be above that of the exec timer
// For the atmega328p, timer 2 has the highest priority, then timer 1, followed by timer 0

// Use 8-bit timer with lower priority for the exec software interrupt
#define EXEC_TIMER_CTRLA    TCCR0A            // Control register A
#define EXEC_TIMER_CTRLB    TCCR0B            // Control register B
#define EXEC_TIMER_IMSK     TIMSK0            // Interrupt mask register
#define EXEC_TIMER_PERIOD   OCR0A             // Output compare register A (period of the timer)
#define EXEC_TIMER_ISR_vect TIMER0_COMPA_vect // Output compare interrupt service routine vector
#define EXEC_TIMER_CS_BM    (1<<CS00)         // Set clock source to F_CPU
#define EXEC_TIMER_WGM_BM   (1<<WGM01)        // Waveform generation mode
#define EXEC_TIMER_OCIE_BM  (1<<OCIE0A)       // Enable output compare match interrupt 

// Use 16-bit timer 1 for the DDA
#define DDA_TIMER_CTRLA    TCCR1A             // Control register A
#define DDA_TIMER_CTRLB    TCCR1B             // Control register B
#define DDA_TIMER_IMSK     TIMSK1             // Interrupt mask register
#define DDA_TIMER_PERIOD   OCR1A              // Output compare register A (period of the timer)
#define DDA_TIMER_ISR_vect TIMER1_COMPA_vect  // Output compare interrupt service routine vector
#define DDA_TIMER_CNT      TCNT1              // Timer count register
#define DDA_TIMER_IFR      TIFR1              // Interrupt flag register
#define DDA_TIMER_CS_BM    (1<<CS10)          // Set clock source to F_CPU
#define DDA_TIMER_OCF_BM   (1<<OCF1A)         // Output compare match flag
#define DDA_TIMER_WGM_BM   (1<<WGM12)         // Waveform generation mode
#define DDA_TIMER_OCIE_BM  (1<<OCIE1A)        // Enable output compare match interrupt

// Use 8-bit timer with higher priority for the load software interrupt
#define LOAD_TIMER_CTRLA    TCCR2A            // Control register A
#define LOAD_TIMER_CTRLB    TCCR2B            // Control register B
#define LOAD_TIMER_IMSK     TIMSK2            // Interrupt mask register
#define LOAD_TIMER_PERIOD   OCR2A             // Output compare register A (period of the timer)
#define LOAD_TIMER_ISR_vect TIMER2_COMPA_vect // Output compare interrupt service routine vector
#define LOAD_TIMER_CS_BM    (1<<CS20)         // Set clock source to F_CPU
#define LOAD_TIMER_WGM_BM   (1<<WGM21)        // Waveform generation mode
#define LOAD_TIMER_OCIE_BM  (1<<OCIE2A)       // Enable output compare match interrupt

#endif  // HARDWARE_H
