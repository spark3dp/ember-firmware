/**
 * @file hardware.h
 */

#ifndef __HARDWARE_REVA3_H__
#define __HARDWARE_REVA3_H__

#define INTERFACE_INTERRUPT A1 //!< Control Panel Interrupt Pin

#define BUTTON1 3 //!< Control Panel Button 1 Pin  
#define BUTTON2 2 //!< Control Panel Button 2Pin

#define OLED_RST    A2 //!< OLED reset pin
#define OLED_DC     A3 //!< OLED DC pin
#define OLED_CS     10 //!< OLED CS pin

#define RING_LEDS 21 //!< ring number of LEDs
#define RING_OE 8 //!< ring OE pin 
#define RING_LATCH 9 //!< ring latch pin
#define RING_CLOCK 7 //!< ring clock pin
#define RING_DATA 6 //!< ring data pin

uint8_t RING_NUMBERING[RING_LEDS]={12,13,14,15,16,17,18,19,20,0,1,2,3,4,5,6,7,8,9,10,11}; //!<Array to map led ring pins to led numbers



#endif
