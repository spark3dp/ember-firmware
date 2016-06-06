//  File:   Logging.cpp
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

#include "Logging.h"

#define PRINT_LEVEL 0


#ifdef ARDUINO
void Logging::print(const char *msg,int level) {
    Serial.println(msg);
}
#else
void Logging::print(const char *msg,int level) {
    cout << msg;
}
#endif

void Logging::init(int level){
    _level = level;
    _numHandlers = 0;
}

void Logging::addHandler(void (*handlerFunction)(const char *,int)){
    
    _logHandlers[_numHandlers++] = handlerFunction;
	if (_numHandlers==MAX_LOG_HANDLERS) _numHandlers=0;

}

void Logging::error(char* msg, ...){
    if (LOG_ERROR <= _level) {
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_ERROR);
        va_end(args);
    }
}

void Logging::error(const __FlashStringHelper* fmsg,...) {
    if (LOG_ERROR <= _level) {
        char msg[MAX_LOG_MSG];
        strncpy_P(msg,(const char*) fmsg,MAX_LOG_MSG);
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_ERROR);
        va_end(args);
    }
}
void Logging::info(char* msg, ...){
    if (LOG_INFO <= _level) {
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_INFO);
        va_end(args);
    }
}

void Logging::info(const __FlashStringHelper* fmsg,...) {
    if (LOG_INFO <= _level) {
        char msg[MAX_LOG_MSG];
        strncpy_P(msg,(const char*) fmsg,MAX_LOG_MSG);
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_INFO);
        va_end(args);
    }
}

void Logging::debug(char* msg, ...){
    if (LOG_DEBUG <= _level) {
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_DEBUG);
        va_end(args);
    }
}


void Logging::debug(const __FlashStringHelper* fmsg,...) {
    if (LOG_DEBUG <= _level) {
        char msg[MAX_LOG_MSG];
        strncpy_P(msg,(const char*) fmsg,MAX_LOG_MSG);
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_DEBUG);
        va_end(args);
    }
}

void Logging::warning(char* msg, ...){
    if (LOG_WARN <= _level) {
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_WARN);
        va_end(args);
    }
}

void Logging::warning(const __FlashStringHelper* fmsg,...) {
    if (LOG_WARN <= _level) {
        char msg[MAX_LOG_MSG];
        strncpy_P(msg,(const char*) fmsg,MAX_LOG_MSG);
        va_list args;
        va_start(args, msg);
        log(msg,args,LOG_WARN);
        va_end(args);
    }
}

void Logging::log(const char *format, va_list args, int level) {

     char msg[MAX_LOG_MSG];
     //char *logMsg = new char[MAX_LOG_MSG];
     char logMsg[MAX_LOG_MSG];

     vsnprintf(msg,MAX_LOG_MSG,format,args);
     va_end(args);


     if (PRINT_LEVEL) {
        switch (level) {
            case LOG_DEBUG:
                snprintf(logMsg,MAX_LOG_MSG,"DEBUG:%s",msg);
                break;
            case LOG_INFO:
                snprintf(logMsg,MAX_LOG_MSG,"INFO: %s",msg);
                break;
            case LOG_WARN:
                snprintf(logMsg,MAX_LOG_MSG,"WARN: %s",msg);
                break;
            case LOG_ERROR:
                snprintf(logMsg,MAX_LOG_MSG,"ERR:  %s",msg);
                break;
        }
     } else {
        snprintf(logMsg,MAX_LOG_MSG,"%s",msg);
    }

     if (_numHandlers==0) {
         //print(logMsg,level);
         print(&logMsg[0],level);
     } else {
         for (int i=0;i<_numHandlers;i++) {
             //_logHandlers[i](logMsg,level);
             _logHandlers[i](&logMsg[0],level);
         }
     }

     //delete logMsg;

     return;
}

Logging Log;
