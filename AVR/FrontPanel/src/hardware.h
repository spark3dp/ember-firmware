#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#if HARDWARE_VERSION == A3
#include "hardware-revA3.h"
#else
#error "Hardware version not supported
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
