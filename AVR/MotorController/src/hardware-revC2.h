#ifndef __HARDWARE_REVC_H__
#define __HARDWARE_REVC_H__

// New pins for Motorboard Rev C.2

#define I2C_ADDRESS 0x10

#define MB_INT 0
#define FAULT_LED 9

// Shared Signals

#define M_SLEEPn A0  // pin 26
#define M_ENABLEn A1 // pin 23
#define M_RESETn A2 // pin 32
#define M_MODE0 12 // pin 16
#define M_MODE1 11 // pin 15
#define M_MODE2 13 // pin 14

// Motor 1 Signals

#define M1_DIR 1 // pin 10
#define M1_STEP 10 // pin 2
#define M1_VREF 3 // pin 9

// Motor 2 Signals

#define M2_DIR 7 // pin 24
#define M2_STEP 6 // pin 17
#define M2_VREF 5 // pin 1

// Limit Switches

#define M1_LIMIT 4
#define M2_LIMIT 2


// Arduino pis for the motors

#define	MOTOR_1_DIR	M1_DIR
#define	MOTOR_1_STEP	M1_STEP
#define	MOTOR_2_DIR	M2_DIR
#define	MOTOR_2_STEP	M2_STEP
#define	MOTOR_3_DIR	99
#define	MOTOR_3_STEP	99

#define	MOTOR_RESET	M_RESETn
#define	MOTOR_MS0	M_MODE0
#define	MOTOR_MS1	M_MODE1
#define	MOTOR_MS2	M_MODE2

#define	Z_MOTOR_STEP	MOTOR_1_STEP
#define Z_MOTOR_DIR	MOTOR_1_DIR

#define	TANK_MOTOR_STEP	MOTOR_2_STEP
#define TANK_MOTOR_DIR	MOTOR_2_DIR




//Arduino pins for the microswitches

#define Z_ENDSTOP	M1_LIMIT
#define TANK_ENDSTOP	M2_LIMIT

// Projector is on pin 13 - this is the internal LED on
//	Arduino.

#define	PROJECTOR		  FAULT_LED
#define	BUSY_LAMP		  FAULT_LED


// The mechanics

#define Z_GearRatio 1UL
#define Rotation_GearRatio 2UL

#define Motor_Steps_Per_Rev 200UL

#define	Z_PITCH              2000UL
#define	Z_STEPS_PER_REV		 Z_GearRatio*Motor_Steps_Per_Rev

#define	TANK_STEPS_PER_REV	 Rotation_GearRatio*Motor_Steps_Per_Rev

#define	Z_STEPS_PER_MM	(Z_STEPS_PER_REV / (Z_PITCH / 1000UL))

// These may need changing, or the steppers may need re-wiring to get
//	the directions correct ...

#define	UP_DIRECTION		  LOW
#define	DOWN_DIRECTION		  HIGH

#define	CLOCKWISE		  LOW
#define	ANTI_CLOCKWISE		  HIGH

#endif
