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

#define UI_INTERRUPT_PIN    (46)   // GPIO 46 P8-16, interrupt from UI board
#define DOOR_INTERRUPT_PIN  (44)   // GPIO 44 P8-12, interrupt from door switch

#define ACK                 ('@')

#define PULSE_PERIOD_SEC    (1)    // period of status updates while printing

#endif	/* HARDWARE_H */

