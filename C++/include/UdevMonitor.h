//  File:   UdevMonitor.h
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

#ifndef UDEVMONITOR_H
#define	UDEVMONITOR_H

#include "IResource.h"

// forward declarations
struct udev;
struct udev_monitor;

class UdevMonitor : public IResource
{
public:
    UdevMonitor(const std::string& subsystem, const std::string& deviceType,
            const std::string& action);
    ~UdevMonitor();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    bool QualifyEvents(uint32_t events) const;
    EventDataVec Read();

private:
    // this class owns a file based resource
    // disable copy construction and copy assignment
    UdevMonitor(const UdevMonitor&);
    UdevMonitor& operator=(const UdevMonitor&);
    void TearDown();

private:
    udev* _pUdev;
    udev_monitor* _pMonitor;
    int _fd;
    const std::string _action;
};

#endif    // UDEVMONITOR_H

