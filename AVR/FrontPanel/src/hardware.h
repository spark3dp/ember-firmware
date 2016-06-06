//  File:   hardware.h
//  Hardware definition checks
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

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#if HARDWARE_VERSION == A3
#include "hardware-revA3.h"
#else
#error "Hardware version not supported"
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Hardware define checks
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef BUTTON1
#error "BUTTON1 must be defined in hardware.h"
#endif

#ifndef BUTTON2
#error "BUTTON2 must be defined in hardware.h"
#endif

#ifndef RING_OE
#error "RING_OE must be defined in hardware.h"
#endif

#ifndef OLED_RST
#error "OLED_RST must be defined in hardware.h"
#endif

#ifndef OLED_CS
#error "OLED_CS must be defined in hardware.h"
#endif

#ifndef OLED_DC
#error "OLED_DC must be defined in hardware.h"
#endif

#ifndef INTERFACE_INTERRUPT
#error "INTERFACE_INTERRUPT must be defined in hardware.h"
#endif


#endif
