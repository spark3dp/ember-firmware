//  File:   Signals.h
//  Provides application with notifications of signals from operating system
//  via signalfd, exposed as a pollable resource
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

#ifndef SIGNALS_H
#define	SIGNALS_H

#include "IResource.h"

class Signals : public IResource
{
public:
    Signals();
    ~Signals();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    bool QualifyEvents(uint32_t events) const;
    EventDataVec Read();
 
private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    Signals(const Signals&);
    Signals& operator=(const Signals&);

private:
    size_t _dataSize;
    int _fd;
};

#endif    // SIGNALS_H

