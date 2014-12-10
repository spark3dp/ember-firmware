/**
 * @file Logging.h
 * @brief Electric Echidna Logging Library
 * @copyright (c) 2014, Electric Echidna Ltd All Rights Reserved
 *
 * Used under license.  Refer to your Electric Echidna project manager
 *
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

#if AVR
#include "Arduino.h"
#include <avr/pgmspace.h>
#define endl "\r\n"
#endif



/**
 * Log types
 *
 */

enum {
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_OFF
};

typedef void (*voidLogFuncPtr)(const char*, int);

#define MAX_LOG_MSG 200
#define MAX_LOG_HANDLERS 5


class Logging {
protected:
    int _level;
    int _numHandlers;

public:
    /*! 
	 * default Constructor
	 */
    Logging(){_level = LOG_DEBUG;} 
	
    /** 
	* Initializing, must be called as first.
	* \param void
	* \return void
	*
	*/
	void init(int level);
	
    /**
	* Output an error message. Output message contains
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/
    void error(char* msg, ...);
    void error(const __FlashStringHelper *fmsg, ...);
	
    /**
	* Output an info message. Output message contains
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

   void info(char* msg, ...);
   void info(const __FlashStringHelper *fmsg, ...);
	
    /**
	* Output an debug message. Output message contains
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

    void debug(char* msg, ...);
    void debug(const __FlashStringHelper *fmsg, ...);
	
    /**
	* Output a warning message. Output message contains
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

    void warning(char* msg, ...);   
    void warning(const __FlashStringHelper *fmsg, ...);

	void addHandler(void (*logFunc)(const char*,int));

    void setLevel(int level) { _level = level; }

protected:
    void log(const char* format, va_list args, int ltype);
    voidLogFuncPtr _logHandlers[MAX_LOG_HANDLERS];
    void print(const char* msg, int level);
};

extern Logging Log;
#endif




