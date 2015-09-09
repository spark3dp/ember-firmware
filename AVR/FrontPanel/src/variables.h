//  File:   variables.h
//  Customisable variables
//
//  This file is part of the Ember Front Panel firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Evan Davey  <http://www.ekidna.io/ember/>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <Logging.h> //For LOG_LEVEL constants

//-------CHANGE BELOW THIS LINE

//#define DEBUG 1

#define BUTTON_HOLD_TIME_MS 3000 //!< Time (in milisceonds) button must be depressed to register a held event
#define DEBOUNCE_TIME_MS 200 //!< Button debounce time

#define RING_ON_PWM 4095 //!< Ring Led ON PWM level

#define LOG_BAUD 9600 //!< BAUD rate for log messages
#define LOG_LEVEL LOG_DEBUG //!< Log level (LOG_OFF,LOG_DEBUG,LOG_ERROR,LOG_WARNING,LOG_INFO)

#define INTERFACE_INTERRUPT_TIME 200 //!< Interrupt high time
#define INTERFACE_ADDRESS 0x11 //!< Control Panel I2C address


//=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Animation Timings
//=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define A1_LOOP_MS 12000 //!< Time in ms to complete a CW loop animation 1 

#define A2_LOOP_MS 6000 //!< Time in ms to complete a CW loop animation 2

#define A3_FADE_ON_MS 2000 //!< Time in ms to complete fade on animation 3
#define A3_FADE_ON_STEP 200 //!< Fade on step increment animation 3
#define A3_ON_MS 0 //!< Pause on time ms animation 3
#define A3_OFF_MS 2000 //!< Turn to turn off CW in animation 3

#define A4_LOOP_MS 2000 //!< Time in ms to complete opposing loop in animation 4

#define A5_FADE_ON_MS 2000 //!< Time in ms to complete fade on animation 5 and 8
#define A5_FADE_ON_STEP 200 //!< Fade on step increment animation 5 and 8
#define A5_ON_MS 0 //!< Pause on time in ms animation 5 and 8
#define A5_FADE_OFF_MS 2000 //!< Time in ms to complete fade off animation 5 and 8
#define A5_FADE_OFF_STEP 200 //!< Fade off step increment animation 5 and 8

#define A6_ON_MS 5000 //!< Time in ms to pause on in animation 6
#define A6_OFF_MS 2000 //!< Time in ms to turn off CW in animation 6

#define A7_ON_MS 5000 //!< Time in ms to pause on in animation 7
#define A7_OFF_MS 2000 //!< Time in ms to turn off vertically in animation 7

#define A11_FALL_MS 2000 //!< Time in ms to turn on/off in animation 11

//-------CHANGE ABOVE THIS LINE

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Variable define checks
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef DEBOUNCE_TIME_MS
#error "DEBOUNCE_TIME_MS must be defined in variables.h"
#endif

#ifndef INTERFACE_INTERRUPT_TIME
#error "INTERFACE_INTERRUPT_TIME must be defined in variables.h"
#endif

#ifndef INTERFACE_ADDRESS
#error "INTERFACE_ADDRESS must be defined in variables.h"
#endif

#ifndef LOG_LEVEL
#error "LOG_LEVEL must be defined in variables.h"
#endif

#ifndef LOG_BAUD
#error "LOG_BAUD must be defined in variables.h"
#endif

#ifndef RING_ON_PWM
#error "RING_ON_PWM must be defined in variables.h"
#endif

#if LOG_LEVEL == LOG_OFF
#warning "Logging disabled"
#endif


#endif
