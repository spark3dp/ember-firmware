/* 
 * File:   PrintEngine.h
 * Author: greener
 *
 * Created on April 1, 2014, 3:09 PM
 */

#ifndef PRINTENGINE_H
#define	PRINTENGINE_H

struct PrinterStatus
{
    //PrintEngineState _state;
    bool _isError;
    int _errorCode;
    char* _errorMessage;
    int _numLayers;
    int _currentLayer;
    int _estimatedSecondsRemaining;
    char* _jobName;  // e.g. base file name for PNGs
    float _temperature;
};

#endif	/* PRINTENGINE_H */

