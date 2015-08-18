/* 
 * File:   UdevMonitor.cpp
 * Author: Jason Lefley
 * 
 * Exposes udev events for a particular subsystem and device type as a pollable
 * resource.
 * 
 * Created on August 18, 2015, 2:18 PM
 */

#include <sys/epoll.h>
#include <libudev.h>

#include "UdevMonitor.h"

/// Constructor
/// Accepts filter parameters to identify udev events of interest
UdevMonitor::UdevMonitor(const std::string& subsystem,
        const std::string& deviceType) :
_udev(udev_new())
{

}

UdevMonitor::~UdevMonitor()
{
}

uint32_t UdevMonitor::GetEventTypes() const
{
    return EPOLLIN;
}

int UdevMonitor::GetFileDescriptor() const
{

}

bool UdevMonitor::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}

ResourceBufferVec UdevMonitor::Read()
{
}
