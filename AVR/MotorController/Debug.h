/*
 * Debug.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG // Don't allow use for non-debug builds
// File including this file can use printf and printf_P(PSTR(...
#include <stdio.h> 
#include <avr/pgmspace.h>

namespace Debug
{
void Initialize();
}
#endif /* DEBUG */

#endif /* DEBUG_H */
