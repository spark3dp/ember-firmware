/* 
 * File:   utils.cpp
 * Author: Richard Greene
 * 
 * Some handy utilities
 *
 * Created on April 17, 2014, 3:16 PM
 */

#include <time.h>
#include <stdio.h>

char msg[256];
/// Format an error message with the given value.
char* FormatError(const char * format, int value)
{
    sprintf(msg, format, value);
    return msg;
}

/// Get the current time in millliseconds
long getMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

