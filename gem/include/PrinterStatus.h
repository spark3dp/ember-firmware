/* 
 * File:   PrinterStatus.h
 * Author: greener
 *
 * Created on April 1, 2014, 3:09 PM
 */

#ifndef PRINTERSTATUS_H
#define	PRINTERSTATUS_H

struct PrinterStatus
{
    const char* _state;
    bool _isError;
    int _errorCode;
    char* _errorMessage;
    int _numLayers;
    int _currentLayer;
    int _estimatedSecondsRemaining;
    char* _jobName;  // e.g. base file name for PNGs
    float _temperature;
};

#endif	/* PRINTERSTATUS_H */

