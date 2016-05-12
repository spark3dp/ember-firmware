//  File:   DRM_DumbBuffer.cpp
//  Encapsulates a DRM dumb buffer.
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

#include "DRM_DumbBuffer.h"

#include <stdexcept>

#include "DRM_Device.h"
#include "DRM_Connector.h"
#include "Logger.h"

DRM_DumbBuffer::DRM_DumbBuffer(const DRM_Device& drmDevice,
                               const DRM_Connector& drmConnector,
                               int width, int height, int bitsPerPixel) :
_drmDeviceFileDescriptor(drmDevice.GetFileDescriptor()),
_modeInfo(drmConnector.GetModeInfo(width, height)),
_bitsPerPixel(bitsPerPixel)
{
    // Verify that the device supports dumb buffering.
    // Dumb buffering allows usage of simple memory-mapped buffers without
    // involving driver specific code.
    if(!drmDevice.SuportsDumbBuffer())
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR,
                                                  DrmNoDumbBufferSupport));
    }

    std::memset(&_createRequest, 0, sizeof(_createRequest));

    _createRequest.width = _modeInfo.hdisplay;
    _createRequest.height = _modeInfo.vdisplay;
    _createRequest.bpp = bitsPerPixel;

    if (drmIoctl(_drmDeviceFileDescriptor, DRM_IOCTL_MODE_CREATE_DUMB,
                 &_createRequest) < 0)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantCreateDumbBuffer));
    }
}

DRM_DumbBuffer::~DRM_DumbBuffer()
{
    drm_mode_destroy_dumb destroyRequest;
    std::memset(&destroyRequest, 0, sizeof(destroyRequest));
    destroyRequest.handle = _createRequest.handle;
    drmIoctl(_drmDeviceFileDescriptor, DRM_IOCTL_MODE_DESTROY_DUMB,
             &destroyRequest);
}

uint32_t DRM_DumbBuffer::GetPitch() const
{
    return _createRequest.pitch;
}

uint32_t DRM_DumbBuffer::GetHandle() const
{
    return _createRequest.handle;
}

uint64_t DRM_DumbBuffer::GetSize() const
{
    return _createRequest.size;
}

const drmModeModeInfo& DRM_DumbBuffer::GetModeInfo() const
{
    return _modeInfo;
}

int DRM_DumbBuffer::GetBitsPerPixel() const
{
    return _bitsPerPixel;
}

uint16_t DRM_DumbBuffer::GetWidth() const
{
    return _modeInfo.hdisplay;
}

uint16_t DRM_DumbBuffer::GetHeight() const
{
    return _modeInfo.vdisplay;
}
