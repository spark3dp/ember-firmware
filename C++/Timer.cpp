/* 
 * File:   Timer.cpp
 * Author: Jason Lefley
 *
 * Wrapper for system provided timer resource
 * 
 * Created on August 12, 2015, 12:00 PM
 */

#include <sys/timerfd.h>
#include <sys/epoll.h> 
#include <stdexcept>
#include <iostream> // TODO: remove when cerr is removed
#include <stdlib.h> // TODO: remove when exit is removed

#include "Timer.h"

Timer::Timer() :
_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)),
_dataSize(sizeof(uint64_t)),
_events(EPOLLIN | EPOLLERR | EPOLLET)
{
    // TODO: use exception based error handling
    if (_fd < 0)
    {
        std::cerr << "unable to create timerfd" << std::endl;
        exit(-1);
    }
}

Timer::~Timer()
{
    close(_fd);
}

uint32_t Timer::GetEventTypes() const
{
    return _events;
}

int Timer::GetFileDescriptor() const
{
    return _fd;
}

ResourceBufferVec Timer::Read()
{
    ResourceBufferVec buffers;
    char data[_dataSize];

    lseek(_fd, 0, SEEK_SET);
    
    if (read(_fd, data, _dataSize) == _dataSize)
        buffers.push_back(ResourceBuffer(data, _dataSize));

    return buffers;
}

/*
 * Clear the timer
 */
void Timer::Clear() const
{
    // Zeroing the timer stops it
    Start(0.0);
}

/*
 * Return the time remaining until expiration in seconds
 */
double Timer::GetRemainingTimeSeconds() const
{
    itimerspec timerValue;

    if (timerfd_gettime(_fd, &timerValue) == -1)
        throw std::runtime_error("unable to read timer");

    return timerValue.it_value.tv_sec + timerValue.it_value.tv_nsec * 1e-9;
}

/*
 * Set the timer expiration in seconds and start the timer
 * This configures the timer to operate for a single cycle, not to
 * repeat periodically
 */
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
    return _events & events;
}
