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

// corresponding BeagleBone Black (BBB; connector pins shown in comments 

// I2C ports 
constexpr int I2C2_PORT         = 2; // I2C2, BBB P9 pins 19 & 20
constexpr int I2C1_PORT         = 1; // I2C1, BBB P9 pins 17 & 18
constexpr int I2C0_PORT         = 0; // I2C0 

constexpr int MOTOR_SLAVE_ADDRESS = 0x10; // slave address for motor controller
// interrupt from motor controller
constexpr int MOTOR_INTERRUPT_PIN = 60;  // GPIO1_28, BBB P9 pin 12 
constexpr int FP_SLAVE_ADDRESS    = 0x11; // slave address for front panel
// command register address for front panel
constexpr int FP_COMMAND          = 0x20;
// button status register for front panel, reading it clears button bits
constexpr int BTN_STATUS          = 0x30;
// display status register for front panel
constexpr int DISPLAY_STATUS      = 0x31;
// interrupt from front panel
constexpr int FP_INTERRUPT_PIN    = 30;   // GPIO0_30, BBB P9 pin 11 

// front panel status register values 
constexpr int BTN1_PRESS         = 0x01; // press of button 1 
constexpr int BTN1_HOLD          = 0x02; // hold of button 1 
constexpr int BTN2_PRESS         = 0x04; // press of button 2 
constexpr int BTN2_HOLD          = 0x08; // hold of button 2 
// press of buttons 1 & 2
constexpr int BTNS_1_AND_2_PRESS = BTN1_PRESS | BTN2_PRESS;
// hold  of buttons 1 & 2
constexpr int BTNS_1_AND_2_HOLD  = BTN1_HOLD  | BTN2_HOLD;
// indicates when commands cannot be sent
constexpr int FP_BUSY            = 0x10;
// there was an error on the front panel
constexpr int ERROR_STATUS       = 0xFF;

// front panel commands
constexpr int CMD_START           = 0x98;  // Frame start command
constexpr int CMD_END             = 0x99;  // Frame end command
constexpr int CMD_SYNC            = 0x97;  // Sync command
constexpr int CMD_RESET           = 0x96;  // Reset command
constexpr int CMD_RING            = 0x01;  // Put in ring command mode
constexpr int CMD_OLED            = 0x02;  // Put in OLED display command mode
constexpr int CMD_RING_OFF        = 0x01;  // Turn ring off
constexpr int CMD_RING_SEQUENCE   = 0x02;  // Start a ring sequence (0 to stop)
constexpr int CMD_RING_LED        = 0x03;  // Set a ring LED to given value
constexpr int CMD_RING_LEDS       = 0x04;  // Set all ring LEDS to given value
constexpr int CMD_OLED_SETTEXT    = 0x01;  // Set OLED display text flush left
constexpr int CMD_OLED_CENTERTEXT = 0x06;  // Set OLED display text centered on x
constexpr int CMD_OLED_RIGHTTEXT  = 0x07;  // Set OLED display text flush right
constexpr int CMD_OLED_CLEAR      = 0x02;  // Clear OLED display
constexpr int CMD_OLED_ON         = 0x03;  // Turn OLED display on
constexpr int CMD_OLED_OFF        = 0x04;  // Turn OLED display off
constexpr int CMD_SLEEP           = 0x03;  // Set screensaver awake time
// maximum string length for front panel's I2C buffer
constexpr int MAX_OLED_STRING_LEN = 20; 
// numbr of LEDs in the ring around the OLED display
constexpr int NUM_LEDS_IN_RING    = 21;

// inputs read directly 
constexpr int DOOR_SENSOR_PIN      = 47; // GPIO1_15
constexpr int ROTATION_SENSOR_PIN  = 27; // GPIO0_27
// GPIO1_13 connects to front panel button 2 
constexpr int BUTTON2_DIRECT       = 45; 
// GPIO0_23 connects to fan 1
constexpr int FAN_1_PIN            = 23;
// GPIO0_22 connects to fan 2
constexpr int FAN_2_PIN            = 22;
// GPIO1_19 connects to fan 3
constexpr int FAN_3_PIN            = 51;

// I2C interface to projector
// Note: projector register addresses must have their msb set to enable writing,
// except for those used when in Programming Mode
constexpr int PROJECTOR_WRITE_BIT             = 0x80;
// and when reading, they must first indicate their readiness:
constexpr int PROJECTOR_READY_STATUS           = 0x01;
// slave address for projector
constexpr int PROJECTOR_SLAVE_ADDRESS          = 0x1a;
// projector firmware version register
constexpr int PROJECTOR_FW_VERSION_REG         = 0x11;
// current firmware version numbers
constexpr int CURRENT_PROJECTOR_FW_MAJ_VERSION = 0x03;
constexpr int CURRENT_PROJECTOR_FW_MIN_VERSION = 0x00;
// hardware status register
constexpr int PROJECTOR_HW_STATUS_REG          = 0x20;
// hardware status register Internal Initialization error bit mask
constexpr int PROJECTOR_INIT_ERROR             = 1;
// hardware status register bit mask for other errors
constexpr int PROJECTOR_HW_ERROR               = (1 << 2) | (1 << 3) | 
                                                 (1 << 6) | (1 << 7);
// system status register
constexpr int PROJECTOR_SYSTEM_STATUS_REG      = 0x21;
constexpr int PROJECTOR_SYSTEM_MEMORY_FLAG     = 0x01;
// main status register
constexpr int PROJECTOR_MAIN_STATUS_REG        = 0x22;
// main status register Gamma Correction Function Enabled bit mask
constexpr int PROJECTOR_GAMMA_ENABLED          = 1 << 3;
// LED(s) enable register
constexpr int PROJECTOR_LED_ENABLE_REG         = 0x10;
// values to enable or disable the projector's LED(s)
constexpr int PROJECTOR_ENABLE_LEDS            = 0x7; 
constexpr int PROJECTOR_DISABLE_LEDS           = 0x0; 
// LED(s) current register
constexpr int PROJECTOR_LED_CURRENT_REG        = 0x4B;
// PWM polarity register
constexpr int PROJECTOR_LED_PWM_POLARITY_REG   = 0x0B;
// though the datasheet says a value of 0 provides normal PWM polarity, it 
// actually appears to provide reversed polarity
constexpr int PROJECTOR_PWM_POLARITY_NORMAL    = 0x01; 
// gamma correction register
constexpr int PROJECTOR_GAMMA                  = 0x31; 
constexpr int PROJECTOR_GAMMA_DISABLE          = 0x0; 
// Display Mode Selection register
constexpr int PROJECTOR_DISPLAY_MODE_REG       = 0x69;
constexpr int PROJECTOR_VIDEO_MODE             = 0x00; 
constexpr int PROJECTOR_PATTERN_MODE           = 0x01; 
// Pattern Display Data Input Source register
constexpr int PROJECTOR_PATTERN_SOURCE_REG     = 0x6F; 
// Pattern Trigger Mode Selection register
constexpr int PROJECTOR_PATTERN_TRIGGER_REG    = 0x70;
// Pattern Display LUT Control register
constexpr int PROJECTOR_PATTERN_LUT_CTL_REG    = 0x75;
// Pattern Display LUT Offset Pointer register
constexpr int PROJECTOR_PATTERN_LUT_OFFSET_REG = 0x76; 
// Pattern Display LUT Access Control register
constexpr int PROJECTOR_PATTERN_LUT_ACC_REG    = 0x77; 
// Pattern Display LUT Data register
constexpr int PROJECTOR_PATTERN_LUT_DATA_REG   = 0x78; 
// Pattern Exposure Time and Frame Period register
constexpr int PROJECTOR_PATTERN_TIMES_REG      = 0x66; 
// Pattern Display Start/Stop Pattern Sequence register
constexpr int PROJECTOR_PATTERN_START_REG      = 0x65; 
// though the datasheet says to use 0x10 here, they must have meant b10
constexpr int PROJECTOR_START_PATTERN_SEQ      = 0x02; 
constexpr int PROJECTOR_STOP_PATTERN_SEQ       = 0x00; 
// Validate Data Command register
constexpr int PROJECTOR_VALIDATE_REG           = 0x7D; 
constexpr int PROJECTOR_VALID_DATA             = 0x1F; 
// Input Source Selection register
constexpr int PROJECTOR_SOURCE_SELECT_REG      = 0x00;
constexpr int PROJECTOR_SOURCE_FPD_LINK        = 0x03;
constexpr int PROJECTOR_SOURCE_PARALLEL_24     = 0x08;

// Program Mode register
constexpr int PROJECTOR_PROGRAM_MODE_REG       = 0x30;
// the datasheet gives the opposite values for entering/leaving Program Mode:
constexpr int PROJECTOR_ENTER_PROGRAM_MODE     = 0x01; 
constexpr int PROJECTOR_LEAVE_PROGRAM_MODE     = 0; 

// the following registers and commands may only be used when the projector is
// in Program Mode
constexpr int PROJECTOR_READ_CONTROL_REG       = 0x15;
constexpr int PROJECTOR_GET_MFR_ID             = 0xC; 
constexpr int SUPPORTED_PROJECTOR_MFR_ID       = 0x20; 
constexpr int PROJECTOR_GET_DEVICE_ID          = 0xD; 
constexpr int SUPPORTED_PROJECTOR_DEVICE_ID    = 0x227E;            
constexpr int PROJECTOR_START_ADDRESS_REG      = 0x29;
constexpr int PROJECTOR_DATA_SIZE_REG          = 0x2C;
constexpr int PROJECTOR_DOWNLOAD_DATA_REG      = 0x25;
constexpr int PROJECTOR_CALCULATE_CHECKSUM_REG = 0x26;
constexpr int PROJECTOR_GET_CHECKSUM           = 0x0;
constexpr int PROJECTOR_ERASE_SECTOR_REG       = 0x28;
// the start address of the firmware application (past the bootloader))
constexpr unsigned int APP_START_ADDR = 0x20000;
constexpr unsigned char FLASH_BUSY_STATUS       = 0x08;

// video resolution for video mode
constexpr unsigned int VIDEO_MODE_WIDTH  =  1280;
constexpr unsigned int VIDEO_MODE_HEIGHT =  800;

// video resolution for pattern mode
constexpr unsigned int PATTERN_MODE_WIDTH  =  912;
constexpr unsigned int PATTERN_MODE_HEIGHT =  1140;

// string constants for network connectivity
// first (and only) Ethernet interface
constexpr const char* ETHERNET_INTERFACE = "eth0";
// first (and only) wireless interface
constexpr const char* WIFI_INTERFACE     = "wlan0";
constexpr int WIFI_ACCESS_POINT_MODE     = 3; // Mode:Master

// string constants for GPIO interrupt type definition
constexpr const char* GPIO_INTERRUPT_EDGE_RISING  = "rising";
constexpr const char* GPIO_INTERRUPT_EDGE_BOTH    = "both";
constexpr const char* GPIO_INTERRUPT_EDGE_FALLING = "falling";

// string constants for use with device manager
constexpr const char* UDEV_ACTION_ADD             = "add";
constexpr const char* UDEV_ACTION_REMOVE          = "remove";
constexpr const char* UDEV_SUBSYSTEM_BLOCK        = "block";
constexpr const char* UDEV_DEVTYPE_PARTITION      = "partition";

#endif    // HARDWARE_H

