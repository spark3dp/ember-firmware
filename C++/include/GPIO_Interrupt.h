//  File:   GPIO_Interrupt.h
//  Exposes a hardware interrupt as a pollable resource
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
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

#ifndef GPIO_INTERRUPT_H
#define	GPIO_INTERRUPT_H

#include "IResource.h"

class GPIO_Interrupt : public IResource
{
public:
    GPIO_Interrupt(int inputPin, const std::string& edge);
    ~GPIO_Interrupt();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    void UnExport() const;
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    GPIO_Interrupt(const GPIO_Interrupt&);
    GPIO_Interrupt& operator=(const GPIO_Interrupt&);

private:
    int _fd;
    int _pin;
};

#endif    // GPIO_INTERRUPT_H

