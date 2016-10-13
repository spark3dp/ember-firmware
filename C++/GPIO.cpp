//  File:   GPIO.cpp
//  Abstracts access to a digital GPIO pin.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2016 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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

#include "GPIO.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Filenames.h"
#include "ErrorMessage.h"

GPIO::GPIO(int gpioNumber) :
_gpioNumber(gpioNumber)
{
    std::ofstream exportFile;
    exportFile.open(GPIO_EXPORT);

    if (!exportFile.good())
    {
        throw std::runtime_error(ErrorMessage::Format(GpioExport, 
                                                      _gpioNumber, errno));
    }
    
    exportFile << _gpioNumber;
}

GPIO::~GPIO()
{
    std::ofstream unexportFile;
    unexportFile.open(GPIO_UNEXPORT);

    if (!unexportFile.good())
    {
       std::cerr << ErrorMessage::Format(GpioUnexport, errno) << std::endl;
       return;
    }

    unexportFile << _gpioNumber;
}

void GPIO::SetDirectionOut()
{
    char directionPath[64];
    sprintf(directionPath, GPIO_DIRECTION, _gpioNumber);

    std::ofstream directionFile;
    directionFile.open(directionPath);

    if (!directionFile.good())
    {
       throw std::runtime_error(ErrorMessage::Format(GpioDirection, 
                                                     _gpioNumber, errno));
    }

    directionFile << "out";
}

void GPIO::SetOutputHigh()
{
    char valuePath[64];
    sprintf(valuePath, GPIO_VALUE, _gpioNumber);

    std::ofstream valueFile;
    valueFile.open(valuePath);

    if (!valueFile.good())
    {
       throw std::runtime_error(ErrorMessage::Format(GpioOutput, 
                                                     _gpioNumber, errno));
    }

    valueFile << "1";
}