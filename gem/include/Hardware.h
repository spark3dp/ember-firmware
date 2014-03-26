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

#define ACK ('@')   // acknowledgement character

#endif	/* HARDWARE_H */

