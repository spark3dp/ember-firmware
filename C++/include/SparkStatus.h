/* 
 * File:   SparkStatus.h
 * Author: Richard Greene
 * 
 * Defines the states recognized by the Spark API, as a function of 
 * PrintEngineState and UISubState.
 *
 * Created on March 12, 2015, 12:39 PM
 */

#ifndef SPARKSTATUS_H
#define	SPARKSTATUS_H

#include <string>
#include <map>

#include <PrinterStatus.h>

#define SPARK_STATUS  SparkStatus::GetSparkStatus
#define SPARK_JOB_STATUS  SparkStatus::GetSparkJobStatus

// printer states recognized by Spark API
// (printer nevers sends "offline)
#define SPARK_READY        "ready"
#define SPARK_PRINTING     "printing"
#define SPARK_PAUSED       "paused"
#define SPARK_MAINTENANCE  "maintenance"
#define SPARK_ERROR        "error"
#define SPARK_BUSY         "busy"

// print job states recognized by Spark API
// (printer never sends "successful")
#define SPARK_JOB_RECEIVED  "received"
#define SPARK_JOB_PRINTING  "printing"
#define SPARK_JOB_PAUSED    "paused"
#define SPARK_JOB_CANCELED  "canceled"
#define SPARK_JOB_COMPLETED "completed"
#define SPARK_JOB_FAILED    "failed"
// this state is not defined by the Spark API, but is needed to handle 
/// the case where we have no print data
#define SPARK_JOB_NONE      ""

class SparkStatus 
{
public:
    static std::string GetSparkStatus(PrintEngineState state, 
                                      UISubState substate);
    static std::string GetSparkJobStatus(PrintEngineState state, 
                                         UISubState substate, bool printing);
    static bool Validate(PrintEngineState state, UISubState substate);
    static std::map<PrinterStatusKey, std::string> _stateMap;
    static std::map<PrinterStatusKey, std::string> _jobStateMap;
    static std::map<PrinterStatusKey, std::string> _specialKeys;
    
private:
    
};

#endif	/* SPARKSTATUS_H */

