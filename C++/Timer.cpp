//  File:   Timer.cpp
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

#include <sys/timerfd.h>
#include <sys/epoll.h> 
#include <stdexcept>
#include <cerrno>

#include "Timer.h"
#include "ErrorMessage.h"

Timer::Timer() :
_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
_dataSize(sizeof(uint64_t))
{
    if (_fd < 0)
        throw std::runtime_error(ErrorMessage::Format(TimerCreate, errno));
}

Timer::~Timer()
{
    close(_fd);
}

uint32_t Timer::GetEventTypes() const
{
    return EPOLLIN | EPOLLERR | EPOLLET;
}

int Timer::GetFileDescriptor() const
{
    return _fd;
}

EventDataVec Timer::Read()
{
    EventDataVec eventData;
    uint64_t data;

    lseek(_fd, 0, SEEK_SET);
    
    if (read(_fd, &data, _dataSize) == _dataSize)
        eventData.push_back(EventData(data));

    return eventData;
}

// Clear the timer
void Timer::Clear() const
{
    // Zeroing the timer stops it
    Start(0.0);
}

// Return the time remaining until expiration in seconds
double Timer::GetRemainingTimeSeconds() const
{
    itimerspec timerValue;

    if (timerfd_gettime(_fd, &timerValue) == -1)
        throw std::runtime_error("unable to read timer");

    return timerValue.it_value.tv_sec + timerValue.it_value.tv_nsec * 1e-9;
}

// Set the timer expiration in seconds and start the timer
// This configures the timer to operate for a single cycle, not to
// repeat periodically
void Timer::Start(double expirationTimeSeconds) const
{
    itimerspec timerValue;
  
    // Floor the setpoint to get the portion in whole seconds
    int seconds = static_cast<int>(expirationTimeSeconds);

    // Subtract off the whole seconds portion to get the remaining portion
    // Convert to nanoseconds
    int nanoSeconds = static_cast<int>(1e9 * (expirationTimeSeconds - seconds));
    
    timerValue.it_value.tv_sec = seconds;
    timerValue.it_value.tv_nsec = nanoSeconds;

    // Set interval to zero to disable periodicity 
    timerValue.it_interval.tv_sec = 0;
    timerValue.it_interval.tv_nsec = 0;
       
    // Set relative timer (flags argument = 0)
    if (timerfd_settime(_fd, 0, &timerValue, NULL) == -1)
        throw std::runtime_error("unable to set timer");
}

bool Timer::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}
