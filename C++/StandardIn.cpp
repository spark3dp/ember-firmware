/* 
 * File:   StandardIn.cpp
 * Author: Jason Lefley
 * 
 * Created on August 11, 2015, 11:22 AM
 */

#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>

#include "StandardIn.h"

StandardIn::StandardIn() :
_events(EPOLLIN | EPOLLERR | EPOLLPRI)
{
}

StandardIn::~StandardIn()
{
}

uint32_t StandardIn::GetEventTypes() const
{
    return _events;
}

int StandardIn::GetFileDescriptor() const
{
    return STDIN_FILENO;
}

/*
 * Reading from this resource returns a single line
 */
ResourceBufferVec StandardIn::Read()
{
    ResourceBuffer line;
    std::getline(std::cin, line);
    ResourceBufferVec buffers;
    buffers.push_back(line);
    return buffers;
}

bool StandardIn::QualifyEvents(uint32_t events) const
{
    return _events & events;
}

