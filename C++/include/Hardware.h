/* 
 * File:   Hardware.h
 * Author: Richard Greene
 * Definitions of pins and other hardware-related items
 * Created on March 14, 2014, 9:14 AM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#define I2C_PORT            (1) // corresponds to I2C2 on BBB, P9 pins 19 & 20

#define MOTOR_SLAVE_ADDRESS (0x10) // slave address for motor board
#define MOTOR_COMMAND       (0x20) // command register address for motor board
#define MOTOR_STATUS        (0x30) // status register address for motor board
#define MOTOR_INTERRUPT_PIN (45)   // GPIO 45 P8-11, interrupt from motor board

#define UI_SLAVE_ADDRESS    (0x11) // slave address for UI board
#define UI_COMMAND          (0x20) // command register address for UI board
#define UI_STATUS           (0x30) // status register address for UI board
#define UI_INTERRUPT_PIN    (46)   // GPIO 46 P8-16, interrupt from UI board

// status register values common to UI & motor board 
#define ERROR_STATUS        (0xFF) // there was an error on the board

// UI board status register values for various button presses and/or holds
#define BTN1_PRESS          (0x01) // press of button 1 
#define BTN1_HOLD           (0x02) // hold of button 1 
#define BTN2_PRESS          (0x04) // press of button 2 
#define BTN2_HOLD           (0x08) // hold of button 2 
#define BTN3_PRESS          (0x10) // press of button 3 
#define BTN3_HOLD           (0x20) // hold of button 3 
#define BTNS_1_AND_2_PRESS  (BTN1_PRESS | BTN2_PRESS)  // press of buttons 1 & 2

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

#define DOOR_INTERRUPT_PIN  (44)   // GPIO 44 P8-12, interrupt from door switch

#endif	/* HARDWARE_H */

