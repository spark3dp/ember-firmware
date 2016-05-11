//  File:   DRM_Encoder.cpp
//  Encapsulates a DRM encoder.
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

#include "DRM_Encoder.h"

// TODO: remove
#include <iostream>

#include <stdexcept>

#include "DRM_Device.h"
#include "DRM_Connector.h"
#include "Logger.h"

// TODO: see what happens if we pass 0 as the encoderId
DRM_Encoder::DRM_Encoder(const DRM_Device& drmDevice,
                         const DRM_Connector& drmConnector)
{
    std::cout << "retriving drm encoder" << std::endl;
    _pEncoder = drmModeGetEncoder(drmDevice.GetFileDescriptor(),
                                  drmConnector.GetEncoderId());

    if (!_pEncoder)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantRetrieveEncoder));
    }
}

DRM_Encoder::~DRM_Encoder()
{
    std::cout << "freeing drm encoder" << std::endl;
    drmModeFreeEncoder(_pEncoder);
}

uint32_t DRM_Encoder::GetCrtcId() const
{
    return _pEncoder->crtc_id;
}
