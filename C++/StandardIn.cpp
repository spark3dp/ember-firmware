//  File:   StandardIn.cpp
//  Exposes stdin as a pollable resource
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

#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>

#include "StandardIn.h"

StandardIn::StandardIn()
{
}

StandardIn::~StandardIn()
{
}

uint32_t StandardIn::GetEventTypes() const
{
    return EPOLLIN | EPOLLERR | EPOLLPRI;
}

int StandardIn::GetFileDescriptor() const
{
    return STDIN_FILENO;
}

// Reading from this resource returns a single line
EventDataVec StandardIn::Read()
{
    std::string line;
    std::getline(std::cin, line);
    EventDataVec eventData;
    eventData.push_back(EventData(line));
    return eventData;
}

bool StandardIn::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}

