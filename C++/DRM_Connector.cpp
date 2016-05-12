//  File:   DRM_Connector.cpp
//  Encapsulates a DRM connector. Connectors are essentially pipelines to
//  connected displays.
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

#include "DRM_Connector.h"

#include <stdexcept>

#include "DRM_Device.h"
#include "Logger.h"

DRM_Connector::DRM_Connector(const DRM_Device& drmDevice, uint32_t id)
{
    _pConnector = drmModeGetConnector(drmDevice.GetFileDescriptor(), id);
    
    if (!_pConnector)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantRetrieveConnector));
    }
}

DRM_Connector::~DRM_Connector()
{
    drmModeFreeConnector(_pConnector);
}

bool DRM_Connector::IsConnected() const
{
    return _pConnector->connection == DRM_MODE_CONNECTED;
}

uint32_t DRM_Connector::GetEncoderId() const
{
    return _pConnector->encoder_id;
}

uint32_t DRM_Connector::GetId() const
{
    return _pConnector->connector_id;
}

const drmModeModeInfo& DRM_Connector::GetModeInfo(int width, int height) const
{
    for (int i = 0; i < _pConnector->count_modes; i++)
    {
        if (_pConnector->modes[i].hdisplay == width &&
            _pConnector->modes[i].vdisplay == height)
        {
            return _pConnector->modes[i];
        }
    }

    throw std::runtime_error(Logger::LogError(LOG_ERR, DrmModeNotAvailable));
}