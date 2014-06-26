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
#include <ctype.h>
#include <string>

/// Get the current time in millliseconds
long GetMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

long startTime = 0;

/// Start the stopwatch timer
void StartStopwatch()
{
    startTime = GetMillis();    
}

/// Stop the stopwatch and return its time in millliseconds
long StopStopwatch()
{
    return GetMillis() - startTime;
}

/// Convert the given string to upper case, and terminate it at whitespace
char* CmdToUpper(char* cmd)
{
    char* p = cmd;
    while(*p != 0)
    {
        *p++ = toupper(*p);
        if(isspace(*p))
            *p = 0;
    }
    return cmd;
}


/// Replace all instances of the oldVal in str with the newVal
std::string Replace(std::string str, const char* oldVal, const char* newVal)
{
    int pos = 0; 
    while((pos = str.find(oldVal)) != std::string::npos)
         str.replace(pos, 1, newVal); 
    
    return str;
}