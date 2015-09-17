//  File:   Logging.h
//  Electric Echidna Logging Library
//
//  This file is part of the Ember Front Panel firmware.
//
//  Copyright 2015 Ekidna (Electric Echidna Ltd) <http://www.ekidna.io/ember/>
//    
//  Authors:
//  Evan Davey  
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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




