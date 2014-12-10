#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "variables.h"

#ifndef HARDWARE_VERSION
#error "HARDWARE_VERSION must be defined"
#else
#if HARDWARE_VERSION == C2
#include "hardware-revC2.h"
#elif HARDWRAE_VERSION == B1
#include "hardware-revB1.h"
#else
#error "Hardware version not supported
#endif
#endif
#endif
