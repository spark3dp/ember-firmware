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

/*
 * Reading from this resource returns a single line
 */
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

