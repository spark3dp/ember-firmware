#include <string.h> // memcpy

#include "Utils.h"
#include "MachineDefinitions.h"

/*
 * Return minimum of specified floating point values
 */

float Min4(float x1, float x2, float x3, float x4)
{
    float min = x1;
    if (x2 < min) min = x2;
    if (x3 < min) min = x3;
    if (x4 < min) return x4;
    return min;
}

/*
 * Copy AXES_COUNT floating point values fom src to dst
 */

void CopyAxisVector(float dst[], const float src[]) 
{
    memcpy(dst, src, sizeof(float) * AXES_COUNT);
}

/*
 * Copy AXES_COUNT unsigned 8-bit integer values from src to dst
 */

void CopyAxisVector(uint8_t dst[], const uint8_t src[])
{
    memcpy(dst, src, sizeof(uint8_t) * AXES_COUNT);
}
