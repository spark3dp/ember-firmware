//  File:   Signals.cpp
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

#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <stdexcept>
#include <sstream>
#include <cerrno>

#include "Signals.h"
#include "ErrorMessage.h"

// Constructor, sets up signalfd with signals of interest
Signals::Signals() :
_dataSize(sizeof(signalfd_siginfo)),
_fd(-1)
{
    // Listen for INT, TERM, and HUP
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);

    // Block default handling of masked signals
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
        throw std::runtime_error(ErrorMessage::Format(SignalMask, errno));

    _fd = signalfd(-1, &mask, 0);
    
    if (_fd < 0)
        throw std::runtime_error(ErrorMessage::Format(SignalfdCreate, errno));

}

Signals::~Signals()
{
    close(_fd);
}

uint32_t Signals::GetEventTypes() const
{
    return EPOLLIN | EPOLLERR | EPOLLET;
}

int Signals::GetFileDescriptor() const
{
    return _fd;
}

// Read information about the signals triggering the event
// Send out data as strings for consistency with other resource data
EventDataVec Signals::Read()
{
    signalfd_siginfo fdsi;
    EventDataVec eventData;

    if (read(_fd, &fdsi, _dataSize) == _dataSize)
    {
        eventData.push_back(EventData(fdsi.ssi_signo));
    }

    return eventData;
}

bool Signals::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}
