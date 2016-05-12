//  File:   FrameBuffer.cpp
//  Encapsulates access to frame buffer through DRM, providing interface to
//  video hardware
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
//  Richard Greene
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

#include "FrameBuffer.h"

#include <Magick++.h>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>

#include "Logger.h"
#include "Filenames.h"

FrameBuffer::FrameBuffer(int width, int height) :
_drmDevice(DRM_DEVICE_NODE),
_drmResources(_drmDevice),
_drmConnector(_drmDevice, _drmResources.GetConnectorId(0)),
_drmEncoder(_drmDevice, _drmConnector),
_drmDumbBuffer(_drmDevice, _drmConnector, width, height, 32),
_drmFrameBuffer(_drmDevice, _drmDumbBuffer, 24),
_image(width * height)
{
    std::cout << "Selecting " << _drmDumbBuffer.GetWidth() << " x " <<
            _drmDumbBuffer.GetHeight() << " as video resolution" << std::endl;
    
    // Check for a connected display.
    if (!_drmConnector.IsConnected())
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR,
                                                  DrmConnectorNotConnected));
    }
 
    // Perform mode setting.
    uint32_t connectorId = _drmConnector.GetId();
    drmModeModeInfo modeInfo = _drmDumbBuffer.GetModeInfo();
    if (drmModeSetCrtc(_drmDevice.GetFileDescriptor(), _drmEncoder.GetCrtcId(),
                       _drmFrameBuffer.GetId(), 0, 0, &connectorId, 1,
                       &modeInfo) < 0)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantSetCrtc));
    }
    
    // Prepare buffer for memory mapping.
    drm_mode_map_dumb mapRequest;
    std::memset(&mapRequest, 0, sizeof(mapRequest));
    mapRequest.handle = _drmDumbBuffer.GetHandle();
    if (drmIoctl(_drmDevice.GetFileDescriptor(), DRM_IOCTL_MODE_MAP_DUMB,
                 &mapRequest) < 0)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantPrepareDumbBuffer));
    }

    // Perform actual memory mapping.
    _pFrameBufferMap = static_cast<uint8_t*>(mmap(0, _drmDumbBuffer.GetSize(),
                                             PROT_READ | PROT_WRITE, MAP_SHARED,
                                             _drmDevice.GetFileDescriptor(),
                                             mapRequest.offset));

    if (_pFrameBufferMap == MAP_FAILED)
    {
        throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                  DrmCantMapDumbBuffer));
    }

    // Clear the frame buffer.
    std::memset(_pFrameBufferMap, 0, _drmDumbBuffer.GetSize());
    
}

FrameBuffer::~FrameBuffer()
{
    std::memset(_pFrameBufferMap, 0, _drmDumbBuffer.GetSize());
    munmap(_pFrameBufferMap, _drmDumbBuffer.GetSize());
}

// Copies the green channel from the specified image into an auxiliary buffer
// but does not display the result.
void FrameBuffer::Blit(Magick::Image& image)
{
    image.write(0, 0, _drmDumbBuffer.GetWidth(), _drmDumbBuffer.GetHeight(),
                "G", Magick::CharPixel, _image.data());
}

// Sets all pixels of the frame buffer to the specified value and displays the
// result immediately.
void FrameBuffer::Fill(uint8_t value)
{
    std::memset(_pFrameBufferMap, value, _drmDumbBuffer.GetSize());
}

// Displays the contents of the auxiliary buffer immediately.
void FrameBuffer::Swap()
{
    int pitch = _drmDumbBuffer.GetPitch();
    int width = _drmDumbBuffer.GetWidth();
    int height = _drmDumbBuffer.GetHeight();
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t value = _image[width * y + x];
            *(uint32_t*)&_pFrameBufferMap[pitch * y + x * 4] =
                    (value << 16) | // red
                    (value << 8)  | // green
                     value;         // blue
        }
    }
}