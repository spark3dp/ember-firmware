/* 
 * File:   Hardware.h
 * Author: Richard Greene
 * Definitions of pins and other hardware-related items
 * Created on March 14, 2014, 9:14 AM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#include <MotorCommands.h>

#define I2C_PORT            (1) // corresponds to I2C2 on BBB, P9 pins 19 & 20

#define MOTOR_SLAVE_ADDRESS (0x10) // slave address for motor board
#define MOTOR_INTERRUPT_PIN (60)   // GPIO 60 P9-12, interrupt from motor board

#define UI_SLAVE_ADDRESS    (0x11) // slave address for UI board
#define UI_COMMAND          (0x20) // command register address for UI board
// button status register for UI board, reading it clears button bits
#define BTN_STATUS          (0x30)  
#define DISPLAY_STATUS      (0x31) // display status register for UI board
#define UI_INTERRUPT_PIN    (30)   // GPIO 30 P9-11, interrupt from UI board

// status register values common to UI & motor board 
#define ERROR_STATUS        (0xFF) // there was an error on the board

// UI board status register values 
#define BTN1_PRESS          (0x01) // press of button 1 
#define BTN1_HOLD           (0x02) // hold of button 1 
#define BTN2_PRESS          (0x04) // press of button 2 
#define BTN2_HOLD           (0x08) // hold of button 2 
#define BTNS_1_AND_2_PRESS  (BTN1_PRESS | BTN2_PRESS)  // press of buttons 1 & 2
#define UI_BOARD_BUSY       (0x10) // indicates when commands cannot be sent

// UI board commands
#define CMD_START 0x98 // Frame start command
#define CMD_END 0x99 // Frame end command
#define CMD_SYNC 0x97 // Sync command
#define CMD_RESET 0x96 // Reset command
#define CMD_RING 0x01 // Put in ring command mode
#define CMD_OLED 0x02 // Put in OLED command mode
#define CMD_RING_OFF 0x01 // Turn ring off
#define CMD_RING_SEQUENCE 0x02 // Start a ring sequence (0 to stop)
#define CMD_RING_LED 0x03 // Set a ring LED to given value
#define CMD_RING_LEDS 0x04 // Set all ring LEDS to given value
#define CMD_OLED_SETTEXT 0x01 // Set OLED text
#define CMD_OLED_CENTERTEXT 0x06 //!< Set OLED text centered on x
#define CMD_OLED_RIGHTTEXT 0x07 //!< Set OLED text centered on x
#define CMD_OLED_CLEAR 0x02 // Clear OLED 
#define CMD_OLED_ON 0x03 // Turn OLED on
#define CMD_OLED_OFF 0x04 // Turn OLED off
// maximum string length for front panel'sI2C buffer
#define MAX_OLED_STRING_LEN (20) 
#define NUM_LEDS_IN_RING (21)

// motor board status register values 
#define SUCCESS             (0) // motor command completed successfully

#define ACK                 ('@')

// high-level motor board commands
#define HOME_COMMAND ('h')
#define MOVE_TO_START_POSN_COMMAND ('c')
#define SEPARATE_COMMAND ('P')
#define END_PRINT_COMMAND ('R')
#define STOP_MOTOR_COMMAND ('S')
#define LAYER_THICKNESS_COMMAND ("l%04d")
#define SEPARATION_RPM_COMMAND ("s%d")

#define DOOR_INTERRUPT_PIN  (47)   // GPIO 47 P8-15, interrupt from door switch

#endif	/* HARDWARE_H */

