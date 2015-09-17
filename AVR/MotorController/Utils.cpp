//  File: Utils.cpp
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

#include <string.h> // memcpy

#include "Utils.h"
#include "MachineDefinitions.h"

// Return minimum of specified floating point values
float Min4(float x1, float x2, float x3, float x4)
{
    float min = x1;
    if (x2 < min) min = x2;
    if (x3 < min) min = x3;
    if (x4 < min) return x4;
    return min;
}

// Copy AXES_COUNT floating point values fom src to dst
void CopyAxisVector(float dst[], const float src[]) 
{
    memcpy(dst, src, sizeof(float) * AXES_COUNT);
}

// Copy AXES_COUNT unsigned 8-bit integer values from src to dst
void CopyAxisVector(uint8_t dst[], const uint8_t src[])
{
    memcpy(dst, src, sizeof(uint8_t) * AXES_COUNT);
}
