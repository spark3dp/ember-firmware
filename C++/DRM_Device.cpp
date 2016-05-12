//  File:   DRM_Device.cpp
//  Encapsulates a DRM device.
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

#include "DRM_Device.h"

#include <fcntl.h>
#include <stdexcept>
#include <xf86drm.h>

#include "Logger.h"

// Open the DRM device.
DRM_Device::DRM_Device(const std::string& deviceNode)
{
    _fd = open(deviceNode.c_str(), O_RDWR | O_CLOEXEC);

    if (_fd < 0)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantOpenDevice));
    }
}

DRM_Device::~DRM_Device()
{
    close(_fd);
}

// Returns true if device supports dumb buffering, false otherwise.
bool DRM_Device::SuportsDumbBuffer() const
{
    uint64_t supportsDumbBuffer;

    if (drmGetCap(_fd, DRM_CAP_DUMB_BUFFER, &supportsDumbBuffer) < 0)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantGetCapability));
    }

    return supportsDumbBuffer != 0;
}

int DRM_Device::GetFileDescriptor() const
{
    return _fd;
}