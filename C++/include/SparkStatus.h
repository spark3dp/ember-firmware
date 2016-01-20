//  File:   SparkStatus.h
//  Defines the states recognized by the Spark API, as a function of 
//  PrintEngineState and UISubState.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef SPARKSTATUS_H
#define	SPARKSTATUS_H

#include <string>
#include <map>

#include <PrinterStatus.h>

// printer states recognized by Spark API
// (printer nevers sends "offline)
constexpr const char* SPARK_READY         = "ready";
constexpr const char* SPARK_PRINTING      = "printing";
constexpr const char* SPARK_PAUSED        = "paused";
constexpr const char* SPARK_MAINTENANCE   = "maintenance";
constexpr const char* SPARK_ERROR         = "error";
constexpr const char* SPARK_BUSY          = "busy";

// print job states recognized by Spark API
// (printer never sends "successful")
constexpr const char* SPARK_JOB_RECEIVED  = "received";
constexpr const char* SPARK_JOB_PRINTING  = "printing";
constexpr const char* SPARK_JOB_PAUSED    = "paused";
constexpr const char* SPARK_JOB_CANCELED  = "canceled";
constexpr const char* SPARK_JOB_COMPLETED = "completed";
constexpr const char* SPARK_JOB_FAILED    = "failed";
// This state is not defined by the Spark API, but is needed to handle 
// the case where we have no print data.
constexpr const char* SPARK_JOB_NONE      = "";

class SparkStatus 
{
public:
    static std::string GetSparkStatus(PrintEngineState state, 
                                      UISubState substate, 
                                      bool canLoadPrintData);
    static std::string GetSparkJobStatus(PrintEngineState state, 
                                         UISubState substate, bool printing);
    static bool Validate(PrintEngineState state, UISubState substate);
    static std::map<PrinterStatusKey, std::string> _stateMap;
    static std::map<PrinterStatusKey, std::string> _jobStateMap;
    static std::map<PrinterStatusKey, std::string> _specialKeys;  
    
    static PrinterStatusKey Key(PrintEngineState state, UISubState subState)
                            { return PrinterStatus::GetKey(state, subState); }

};

#endif    // SPARKSTATUS_H

