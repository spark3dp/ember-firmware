//  File: Utils.h
//  Utility functions
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from TinyG <https://www.synthetos.com/project/tinyg/>.
//
//  Copyright 2010 - 2015 Alden S. Hart, Jr.
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

float Min4(float x1, float x2, float x3, float x4);
void CopyAxisVector(float dst[], const float src[]);
void CopyAxisVector(uint8_t dst[], const uint8_t src[]);

#ifndef EPSILON
#define EPSILON   0.00001 // rounding error for floats
#endif

#ifndef fp_ZERO
#define fp_ZERO(a) (fabs(a) < EPSILON) // requires math.h to be included in each file used
#endif

#ifndef fp_NOT_ZERO
#define fp_NOT_ZERO(a) (fabs(a) > EPSILON) // requires math.h to be included in each file used
#endif

#ifndef max
#define max(a,b) \
   ({ __typeof__ (a) termA = (a); \
      __typeof__ (b) termB = (b); \
    termA>termB ? termA:termB; })
#endif

#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) termA = (a); \
      __typeof__ (b) termB = (b); \
      termA<termB ? termA:termB; })
#endif

#define MICROSECONDS_PER_MINUTE ((float)60000000)
#define uSec(a) ((float)(a * MICROSECONDS_PER_MINUTE))

#endif  // UTILS_H
