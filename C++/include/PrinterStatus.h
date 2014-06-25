/* 
 * File:   PrinterStatus.h
 * Author: greener
 *
 * Created on April 1, 2014, 3:09 PM
 */

#ifndef PRINTERSTATUS_H
#define	PRINTERSTATUS_H

#include <stddef.h>

/// the possible changes in state
enum StateChange
{
    NoChange,
    Entering,
    Leaving,
};

class PrinterStatus
{
public:    
    const char* _state;
    StateChange _change;
    bool _isError;
    int _errorCode;
    const char* _errorMessage;
    int _numLayers;
    int _currentLayer;
    int _estimatedSecondsRemaining;
    const char* _jobName;  // e.g. base file name for PNGs
    float _temperature;
    
    PrinterStatus() :
    _state(""),
    _change(NoChange),
    _isError(false),
    _errorCode(0),
    _errorMessage(""),
    _numLayers(0),
    _currentLayer(0),
    _estimatedSecondsRemaining(0),
    _jobName(""),
    _temperature(0.0f)
    {}
};

#endif	/* PRINTERSTATUS_H */

