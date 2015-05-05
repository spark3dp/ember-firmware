/*
 * Debug.h
 * Author: Jason Lefley
 * Date  : 2015-05-01
 */

#ifndef DEBUG_H
#define DEBUG_H

// Files including this file can write out to serial with printf
#include <stdio.h> 
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void DebugInitialize();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DEBUG_H */
