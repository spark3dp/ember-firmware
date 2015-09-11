//  File:   Timer.h
//  Exposes a system provided timer as a pollable resource
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

#ifndef TIMER_H
#define	TIMER_H

#include "IResource.h"

class Timer : public IResource
{
public:
    Timer();
    ~Timer();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    void Start(double expirationTimeSeconds) const;
    double GetRemainingTimeSeconds() const;
    void Clear() const;
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    Timer(const Timer&);
    Timer& operator=(const Timer&);

private:
    int _fd;
    size_t _dataSize;
};

#endif    // TIMER_H

