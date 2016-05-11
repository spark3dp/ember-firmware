//  File:   FrameBuffer.h
//  Encapsulates access to frame buffer through SDL, providing interface to
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

#ifndef FRAMEBUFFER_H
#define	FRAMEBUFFER_H

#include <stdexcept>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "ErrorMessage.h"
#include "Logger.h"

class DRM_Device
{
public:
    // Open the DRM device.
    DRM_Device()
    {
        _fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

        if (_fd < 0)
        {
            throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                      DrmCantOpenDevice));
        }
    }

    ~DRM_Device()
    {
        close(_fd);
    }

    // Returns true if device supports dumb buffering, false otherwise.
    bool SuportsDumbBuffer() const
    {
        uint64_t supportsDumbBuffer;
        return drmGetCap(_fd, DRM_CAP_DUMB_BUFFER, &supportsDumbBuffer) >= 0 &&
                supportsDumbBuffer;
    }

    int GetFileDescriptor() const
    {
        return _fd;
    }

private:
    DRM_Device(const DRM_Device&);
    DRM_Device& operator=(const DRM_Device&);

    int _fd;
};

class DRM_Resources
{
public:
    
    // Retrieve resources provided by the DRM device.
    DRM_Resources(const DRM_Device& drmDevice)
    {
        _pResources = drmModeGetResources(drmDevice.GetFileDescriptor());

        if (!_pResources)
        {
            throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                      DrmCantGetResources));
        }
    }

    ~DRM_Resources()
    {
        drmModeFreeResources(_pResources);
    }

    uint32_t GetConnectorId(int connectorIndex) const
    {
        if (connectorIndex < 0 ||
            connectorIndex > _pResources->count_connectors - 1)
        {
            throw std::runtime_error(
                    Logger::LogError(LOG_ERR, DrmConnectorIndexOutOfBounds));

        }
        return _pResources->connectors[connectorIndex];
    }

private:
    drmModeResPtr _pResources;

    DRM_Resources(const DRM_Resources&);
    DRM_Resources& operator=(const DRM_Resources&);
};

class DRM_Connector
{
public:
    // TODO: see what happens if we pass 0 as the connectorId
    DRM_Connector(const DRM_Device& drmDevice, uint32_t id)
    {
        _pConnector = drmModeGetConnector(drmDevice.GetFileDescriptor(), id);
        
        if (!_pConnector)
        {
            throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                      DrmCantRetrieveConnector));
        }
    }

    ~DRM_Connector()
    {
        drmModeFreeConnector(_pConnector);
    }

    bool IsConnected() const
    {
        return _pConnector->connection == DRM_MODE_CONNECTED;
    }

    uint32_t GetEncoderId() const
    {
        return _pConnector->encoder_id;
    }
    
    uint32_t GetId() const
    {
        return _pConnector->connector_id;
    }

    const drmModeModeInfo& GetModeInfo(int width, int height) const
    {
        for (int i = 0; i < _pConnector->count_modes; i++)
        {
            if (_pConnector->modes[i].hdisplay == width &&
                _pConnector->modes[i].vdisplay == height)
            {
                return _pConnector->modes[i];
            }
        }

        throw std::runtime_error(Logger::LogError(LOG_ERR,
                                                  DrmModeNotAvailable));
    }

private:
    drmModeConnectorPtr _pConnector;

    DRM_Connector(const DRM_Connector&);
    DRM_Connector& operator=(const DRM_Connector&);
};

class DRM_Encoder
{
public:
    // TODO: see what happens if we pass 0 as the encoderId
    DRM_Encoder(const DRM_Device& drmDevice, const DRM_Connector& drmConnector)
    {
        _pEncoder = drmModeGetEncoder(drmDevice.GetFileDescriptor(),
                                      drmConnector.GetEncoderId());

        if (!_pEncoder)
        {
            throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                      DrmCantRetrieveEncoder));
        }
    }
    
    ~DRM_Encoder()
    { 
        drmModeFreeEncoder(_pEncoder);   
    }

    uint32_t GetCrtcId() const
    {
        return _pEncoder->crtc_id;
    }

private:
    drmModeEncoderPtr _pEncoder;

    DRM_Encoder(const DRM_Encoder&);
    DRM_Encoder& operator=(const DRM_Encoder&);
};

class DRM_DumbBuffer
{
public:
    DRM_DumbBuffer(const DRM_Device& drmDevice, const DRM_Connector& drmConnector,
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

    uint32_t GetPitch() const
    {
        return _createRequest.pitch;
    }

    uint32_t GetHandle() const
    {
        return _createRequest.handle;
    }

    uint64_t GetSize() const
    {
        return _createRequest.size;
    }
 
    const drmModeModeInfo& GetModeInfo() const
    {
        return _modeInfo;
    }

    int GetBitsPerPixel() const
    {
        return _bitsPerPixel;
    }

    uint16_t GetWidth() const
    {
        return _modeInfo.hdisplay;
    }

    uint16_t GetHeight() const
    {
        return _modeInfo.vdisplay;
    }

    ~DRM_DumbBuffer()
    {
        drm_mode_destroy_dumb destroyRequest;
        std::memset(&destroyRequest, 0, sizeof(destroyRequest));
        destroyRequest.handle = _createRequest.handle;
        drmIoctl(_drmDeviceFileDescriptor, DRM_IOCTL_MODE_DESTROY_DUMB,
                 &destroyRequest);
    }

private:
    DRM_DumbBuffer(const DRM_DumbBuffer&);
    DRM_DumbBuffer& operator=(const DRM_DumbBuffer&);
    
    int _drmDeviceFileDescriptor;
    const drmModeModeInfo& _modeInfo;
    int _bitsPerPixel;
    drm_mode_create_dumb _createRequest;
};

class DRM_FrameBuffer
{
public:
    DRM_FrameBuffer(const DRM_Device& drmDevice, const DRM_DumbBuffer& drmDumbBuffer,
                    int depth) :
    _drmDeviceFileDescriptor(drmDevice.GetFileDescriptor())
    {
        if (drmModeAddFB(_drmDeviceFileDescriptor,
                         drmDumbBuffer.GetWidth(), drmDumbBuffer.GetHeight(),
                         depth, drmDumbBuffer.GetBitsPerPixel(),
                         drmDumbBuffer.GetPitch(), drmDumbBuffer.GetHandle(), &_id) < 0)
        {
            throw std::runtime_error(Logger::LogError(LOG_ERR, errno,
                                                      DrmCantCreateFrameBuffer));
        }
    }
 
    ~DRM_FrameBuffer()
    {
        drmModeRmFB(_drmDeviceFileDescriptor, _id);
    }

    uint32_t GetId() const
    {
        return _id;
    }

private:
    DRM_FrameBuffer(const DRM_FrameBuffer&);
    DRM_FrameBuffer& operator=(const DRM_FrameBuffer&);

    int _drmDeviceFileDescriptor;
    uint32_t _id;
};

#include "IFrameBuffer.h"

class FrameBuffer : public IFrameBuffer
{
public:
    FrameBuffer(int width, int height);
    ~FrameBuffer();
    void Blit(Magick::Image& image);
    void Fill(uint8_t value);
    void Swap();

private:
    DRM_Device _drmDevice;
    DRM_Resources _drmResources;
    DRM_Connector _drmConnector;
    DRM_Encoder _drmEncoder;
    DRM_DumbBuffer _drmDumbBuffer;
    DRM_FrameBuffer _drmFrameBuffer;
    uint8_t* _frameBufferMap;
};


#endif  // FRAMEBUFFER_H

