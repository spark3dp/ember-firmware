//  File:   DRM_Resources.cpp
//  Encapsulates DRM resources.
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

#include "DRM_Resources.h"

#include <stdexcept>

#include "DRM_Device.h"
#include "Logger.h"

// Retrieve resources provided by the DRM device.
DRM_Resources::DRM_Resources(const DRM_Device& drmDevice)
{
    _pResources = drmModeGetResources(drmDevice.GetFileDescriptor());

    if (!_pResources)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantGetResources));
    }
}

DRM_Resources::~DRM_Resources()
{ 
    drmModeFreeResources(_pResources);
}

uint32_t DRM_Resources::GetConnectorId(int connectorIndex) const
{
    if (connectorIndex < 0 ||
        connectorIndex > _pResources->count_connectors - 1)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR,
                                                  DrmConnectorIndexOutOfBounds));
    }
    return _pResources->connectors[connectorIndex];
}
