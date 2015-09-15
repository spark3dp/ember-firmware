//  File:   UdevMonitor.cpp
//  Exposes udev events for a particular subsystem and device type as a pollable
//  resource
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
#include <libudev.h>
#include <cerrno>
#include <stdexcept>

#include "UdevMonitor.h"
#include "ErrorMessage.h"

// Constructor
// Accepts filter parameters to identify udev events of interest
UdevMonitor::UdevMonitor(const std::string& subsystem,
        const std::string& deviceType, const std::string& action) :
_pUdev(udev_new()),
_action(action)
{
    if (!_pUdev)
        throw std::runtime_error(ErrorMessage::Format(UdevCreate, errno));

    // set up a monitor using the specified filters
    _pMonitor = udev_monitor_new_from_netlink(_pUdev, "udev");

    if (!_pMonitor)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(UdevMonitorCreate, 
                                                                        errno));
    }

    if (udev_monitor_filter_add_match_subsystem_devtype(_pMonitor,
            subsystem.c_str(), deviceType.c_str()) < 0)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(UdevAddFilter, errno));
    }

    if (udev_monitor_enable_receiving(_pMonitor) < 0)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(UdevMonitorEnable, 
                                                                        errno));
    }

    // get a file descriptor for polling
    _fd = udev_monitor_get_fd(_pMonitor);

    if (_fd < 0)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(UdevGetFileDescriptor, 
                                                                        errno));
    }
}

UdevMonitor::~UdevMonitor()
{
    TearDown();
}

uint32_t UdevMonitor::GetEventTypes() const
{
    return EPOLLIN;
}

int UdevMonitor::GetFileDescriptor() const
{
    return _fd;
}

bool UdevMonitor::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}

// Read the path of the node corresponding to the activity.
// Returns an empty list if the action reported by udev does not match
// the action filter parameter specified at construction.
EventDataVec UdevMonitor::Read()
{
    EventDataVec eventData;

    // receive the device that triggered the activity
    udev_device* pDevice = udev_monitor_receive_device(_pMonitor);

    if (pDevice)
    {
        const char* action = udev_device_get_action(pDevice);
        const char* node = udev_device_get_devnode(pDevice);
        
        // udev_device_get_action and udev_device_get_devnode may return a
        // NULL pointer
        if (node && std::string(action ? action : "") == _action)
        {
            // this resource encountered activity of interest
            // propagate the activity by returning the node path
            eventData.push_back(EventData(std::string(node)));
        }

        udev_device_unref(pDevice);
    }

    return eventData;
}

// Decrease the ref counts to release the resources.
void UdevMonitor::TearDown()
{
    // dropping the ref to the monitor releases the socket associated with _fd
    // so cleaning up does not require closing the file descriptor

    if (_pMonitor)
        udev_monitor_unref(_pMonitor);

    if (_pUdev)
        udev_unref(_pUdev);
}
