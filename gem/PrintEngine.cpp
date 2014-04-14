/* 
 * File:   PrintEngine.cpp
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#include <PrintEngine.h>
#include <stdio.h>
#include <iostream>

void PrintEngine::SendStatus(const char* stateName)
{
    _status._state = stateName;
    // TODO arrange to update and send actual status
    // for now, just print out what state we're in
    std::cout << _status._state << std::endl; 
}

void PrintEngine::SetNumLayers(int numLayers)
{
    _status._numLayers = numLayers;
    // here we assume the number of layers is only set before starting a print
    _status._currentLayer = 0;
}

int PrintEngine::NextLayer()
{
    return(++_status._currentLayer);   
}

bool PrintEngine::NoMoreLayers()
{
    return _status._currentLayer > _status._numLayers;
}





