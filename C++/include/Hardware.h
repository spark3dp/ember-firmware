//  File:   Hardware.h
//  Definitions of pins and other hardware-related items
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef HARDWARE_H
#define	HARDWARE_H

// corresponding BeagleBone Black (BBB) connector pins shown in comments 

// I2C ports 
#define I2C2_PORT           (1) // I2C2, BBB P9 pins 19 & 20
#define I2C1_PORT           (2) // I2C1, BBB P9 pins 17 & 18
#define I2C0_PORT           (0) // I2C0 

#define MOTOR_SLAVE_ADDRESS (0x10) // slave address for motor controller
// interrupt from motor controller
#define MOTOR_INTERRUPT_PIN (60)  // GPIO1_28, BBB P9 pin 12 

#define FP_SLAVE_ADDRESS    (0x11) // slave address for front panel
#define FP_COMMAND          (0x20) // command register address for front panel
// button status register for front panel, reading it clears button bits
#define BTN_STATUS          (0x30)  
#define DISPLAY_STATUS      (0x31) // display status register for front panel
// interrupt from front panel
#define FP_INTERRUPT_PIN    (30)   // GPIO0_30, BBB P9 pin 11 

// front panel status register values 
#define BTN1_PRESS          (0x01) // press of button 1 
#define BTN1_HOLD           (0x02) // hold of button 1 
#define BTN2_PRESS          (0x04) // press of button 2 
#define BTN2_HOLD           (0x08) // hold of button 2 
#define BTNS_1_AND_2_PRESS  (BTN1_PRESS | BTN2_PRESS)  // press of buttons 1 & 2
#define FP_BUSY             (0x10) // indicates when commands cannot be sent
#define ERROR_STATUS        (0xFF) // there was an error on the front panel

// front panel commands
#define CMD_START           (0x98)  // Frame start command
#define CMD_END             (0x99)  // Frame end command
#define CMD_SYNC            (0x97)  // Sync command
#define CMD_RESET           (0x96)  // Reset command
#define CMD_RING            (0x01)  // Put in ring command mode
#define CMD_OLED            (0x02)  // Put in OLED display command mode
#define CMD_RING_OFF        (0x01)  // Turn ring off
#define CMD_RING_SEQUENCE   (0x02)  // Start a ring sequence (0 to stop)
#define CMD_RING_LED        (0x03)  // Set a ring LED to given value
#define CMD_RING_LEDS       (0x04)  // Set all ring LEDS to given value
#define CMD_OLED_SETTEXT    (0x01)  // Set OLED display text flush left
#define CMD_OLED_CENTERTEXT (0x06)  // Set OLED display text centered on x
#define CMD_OLED_RIGHTTEXT  (0x07)  // Set OLED display text flush right
#define CMD_OLED_CLEAR      (0x02)  // Clear OLED display
#define CMD_OLED_ON         (0x03)  // Turn OLED display on
#define CMD_OLED_OFF        (0x04)  // Turn OLED display off
#define CMD_SLEEP           (0x03)  // Set screensaver awake time
// maximum string length for front panel's I2C buffer
#define MAX_OLED_STRING_LEN (20) 
// numbr of LEDs in the ring around the OLED display
#define NUM_LEDS_IN_RING    (21)

// inputs read directly 
#define DOOR_SENSOR_PIN      (47)   // GPIO1_15
#define ROTATION_SENSOR_PIN  (27)   // GPIO0_27
#define BUTTON2_DIRECT       (45)   // GPIO1_13 connects to front panel button 2 

// I2C interface to projector
// Note: projector register addresses must have their msb set to enable writing 
#define PROJECTOR_SLAVE_ADDRESS         (0x1a) // slave address for projector
#define PROJECTOR_HW_STATUS_REG         (0x20) // hardware status register
#define PROJECTOR_LED_ENABLE_REG        (0x10 | 0x80) // LED(s) enable register
// values to enable or disable the projector's LED(s))
#define PROJECTOR_ENABLE_LEDS           (0x7) 
#define PROJECTOR_DISABLE_LEDS          (0x0) 
#define PROJECTOR_LED_CURRENT_REG       (0x4B | 0x80) // LED(s) current register 
#define PROJECTOR_LED_PWM_POLARITY_REG  (0x0B | 0x80) // PWM polarity register 
// though the datasheet says a value of 0 provides normal PWM polarity, it 
// actually appears to provide reversed polarity
#define PROJECTOR_PWM_POLARITY_NORMAL   (0x01) 

// string constants for network connectivity
#define ETHERNET_INTERFACE ("eth0")  // first (and only) Ethernet interface)
#define WIFI_INTERFACE     ("wlan0") // first (and only) wireless interface)
#define WIFI_ACCESS_POINT_MODE  (3)  // Mode:Master

// string constants for GPIO interrupt type definition
#define GPIO_INTERRUPT_EDGE_RISING  ("rising")
#define GPIO_INTERRUPT_EDGE_BOTH    ("both")
#define GPIO_INTERRUPT_EDGE_FALLING ("falling")

// string constants for use with device manager
#define UDEV_ACTION_ADD             ("add")
#define UDEV_ACTION_REMOVE          ("remove")
#define UDEV_SUBSYSTEM_BLOCK        ("block")
#define UDEV_DEVTYPE_PARTITION      ("partition")

#endif    // HARDWARE_H

