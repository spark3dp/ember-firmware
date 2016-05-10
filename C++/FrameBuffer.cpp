//  File:   FrameBuffer.cpp
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

#include "FrameBuffer.h"

#include <Magick++.h>
#include <stdexcept>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>

FrameBuffer::FrameBuffer(int width, int height) :
_drmResources(_drmDevice),
_drmConnector(_drmDevice, _drmResources.GetConnectorId(0)),
_drmEncoder(_drmDevice, _drmConnector),
_drmDumbBuffer(_drmDevice, _drmConnector, width, height, 32),
_drmFrameBuffer(_drmDevice, _drmDumbBuffer, 24),
_image(width * height * 4)
{
    // Open the DRM device.
//    _fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
//
//    if (_fd < 0)
//    {
//        throw std::runtime_error(ErrorMessage::Format(DrmCantOpenDevice,
//                                                      errno));
//    }
   
    // Verify that the device supports dumb buffering.
    // Dumb buffering allows usage of simple memory-mapped buffers without
    // involving driver specific code.
    // now done in DRM_DumbBuffer constructor
//    if(!_drmDevice.SuportsDumbBuffer())
//    {
//        throw std::runtime_error("DrmNoDumbBufferSupport");
//    }
//    if (drmGetCap(_fd, DRM_CAP_DUMB_BUFFER, &supportsDumbBuffer) < 0 ||
//        !supportsDumbBuffer)
//    {
//        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmNoDumbBufferSupport));
//        throw std::runtime_error("DrmNoDumbBufferSupport");
//    }

    // Retrieve resources provided by the device.
//    drmModeRes* pDRMResources = drmModeGetResources(_fd);
//
//    if (!pDRMResources)
//    {
//        close(_fd);
//        throw std::runtime_error(ErrorMessage::Format(DrmCantGetResources,
//                                                      errno));
//    }

    // Verify presence of a single connector.
    // Connectors are essentially pipelines to connected displays.
    // check performed when calling GetConnectorId on DRM_Resource in initialization list
//    if (_drmResources.GetConnectorCount() != 1)
//    {
//        throw std::runtime_error("DrmUnexpectedConnectorCount");
//    }
//    if (pDRMResources->count_connectors != 1)
//    {
//        drmModeFreeResources(pDRMResources);
////        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmUnexpectedConnectorCount,
//        //        pDRMResources->count_connectors));
//        throw std::runtime_error("DrmUnexpectedConnectorCount");
//    }
    
    // Attempt to the prepare connector for use.
//    drmModeConnector* pDRMConnector =
//            drmModeGetConnector(_fd, pDRMResources->connectors[0]);
//
//    if (!pDRMConnector)
//    {
//        drmModeFreeResources(pDRMResources);
////        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmCantRetrieveConnector));
//        throw std::runtime_error("DrmCantRetrieveConnector");
//    }

//    if (pDRMConnector->connection != DRM_MODE_CONNECTED)
//    {
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
//        close(_fd);
        //throw std::runtime_error(ErrorMessage::Format(DrmConnectorDisconnected));
    // Check for a connected display.
    if (!_drmConnector.IsConnected())
    {
        throw std::runtime_error("DrmConnectorDisconnected");
    }
 
//    if (pDRMConnector->count_modes == 0)
//    {
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
//        close(_fd);
        //throw std::runtime_error(ErrorMessage::Format(DrmNoValidModeForConnector));
    // Check for at least one valid mode.
    // check performed by virtue of searching for matching mode
//    if (_drmConnector.GetModeCount() == 0)
//    {
//        throw std::runtime_error("DrmNoValidModeForConnector");
//    }

    // TODO: print out video parameters ?

    // Find a CRTC/encoder for this connector.
    // A CRTC is a controller that manages which data goes to which connector.
    // Encoders help the CRTC to convert data from a frame buffer into the right
    // format for the chosen connector.

    // Verify that the connector has an associated encoder.
//    if (!pDRMConnector->encoder_id)
//    {
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
//        close(_fd);
//        throw std::runtime_error(ErrorMessage::Format(DrmNoExistingEncoder));
//        throw std::runtime_error("DrmNoExistingEncoder");
//    }

    // Retrieve the encoder.
//    drmModeEncoder* pDRMEncoder =
//            drmModeGetEncoder(_fd, pDRMConnector->encoder_id);
//
//    if (!pDRMEncoder)
//    {
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
////        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmCantRetrieveEncoder));
//        throw std::runtime_error("DrmCantRetrieveEncoder");
//    }

    // Verify that the encoder has an associated CRTC.
//    if (!pDRMEncoder->crtc_id)
//    {
//        drmModeFreeEncoder(pDRMEncoder);   
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
////        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmNoExistingCrtc));
//        throw std::runtime_error("DrmNoExistingCrtc");
//    }

    // Create a dumb buffer.
//    drm_mode_create_dumb createRequest;
//    std::memset(&createRequest, 0, sizeof(createRequest));
//    
//    // TODO: use values from available modes
//    createRequest.width = 1280;
//    createRequest.height = 800;
//    createRequest.bpp = 32;

//    if (drmIoctl(_fd, DRM_IOCTL_MODE_CREATE_DUMB, &createRequest) < 0)
//    {
//        drmModeFreeEncoder(pDRMEncoder);   
//        drmModeFreeConnector(pDRMConnector);
//        drmModeFreeResources(pDRMResources);
////        close(_fd);
//        //throw std::runtime_error(ErrorMessage::Format(DrmCantCreateDumbBuffer,
//        //                                              errno));
//        throw std::runtime_error("DrmCantCreateDumbBuffer");
//    }

    // Create a frame buffer object for the dumb buffer.
//    uint32_t frameBuffer;
//    // TODO: use values from available modes
//    if (drmModeAddFB(_drmDevice.GetFileDescriptor(), width, height, depth,
//                     bitsPerPixel, _drmDumbBuffer.GetPitch(),
//                     _drmDumbBuffer.GetHandle(), &frameBuffer) < 0)
//    {
//        // TODO: cleanup
//        // submit destroy request
////        throw std::runtime_error(ErrorMessage::Format(DrmCantCreateFrameBuffer,
////                                                      errno));
//        throw std::runtime_error("DrmCantCreateFrameBuffer");
//    }

    // Prepare buffer for memory mapping.
//    drm_mode_map_dumb mapRequest;
//    std::memset(&mapRequest, 0, sizeof(mapRequest));
//    mapRequest.handle = _drmDumbBuffer.GetHandle();
//    if (drmIoctl(_drmDevice.GetFileDescriptor(), DRM_IOCTL_MODE_MAP_DUMB,
//                 &mapRequest) < 0)
//    {
//        throw std::runtime_error("DrmCantPrepareFrameBuffer");
//    }

    // Perform actual memory mapping.
//    _frameBufferMap = static_cast<uint8_t*>(mmap(0, _drmDumbBuffer.GetSize(),
//                                            PROT_READ | PROT_WRITE, MAP_SHARED,
//                                            _drmDevice.GetFileDescriptor(),
//                                            mapRequest.offset));
//    if (_frameBufferMap == MAP_FAILED)
//    {
//        throw std::runtime_error("DrmCantMapFrameBuffer");
//    }

    // Clear the frame buffer.
//    std::memset(_frameBufferMap, 0, _drmDumbBuffer.GetSize());

   
    // Perform mode setting.
    // TODO: see what happens if crtc id is zero
    uint32_t connectorId = _drmConnector.GetId();
    drmModeModeInfo modeInfo = _drmDumbBuffer.GetModeInfo();
    if (drmModeSetCrtc(_drmDevice.GetFileDescriptor(), _drmEncoder.GetCrtcId(),
                       _drmFrameBuffer.GetId(), 0, 0, &connectorId, 1,
                       &modeInfo) < 0)
    {
        throw std::runtime_error("DrmCantSetCrtc");
    }
    
    drm_mode_map_dumb mapRequest;
    std::memset(&mapRequest, 0, sizeof(mapRequest));
    mapRequest.handle = _drmDumbBuffer.GetHandle();
    if (drmIoctl(_drmDevice.GetFileDescriptor(), DRM_IOCTL_MODE_MAP_DUMB,
                 &mapRequest) < 0)
    {
        throw std::runtime_error("DrmCantPrepareFrameBuffer");
    }

    // Perform actual memory mapping.
    _frameBufferMap = static_cast<uint8_t*>(mmap(0, _drmDumbBuffer.GetSize(),
                                            PROT_READ | PROT_WRITE, MAP_SHARED,
                                            _drmDevice.GetFileDescriptor(),
                                            mapRequest.offset));

    if (_frameBufferMap == MAP_FAILED)
    {
        throw std::runtime_error("DrmCantMapFrameBuffer");
    }

    // Clear the frame buffer.
    std::memset(_frameBufferMap, 0, _drmDumbBuffer.GetSize());
    
//    // Perform mode setting.
//    if (drmModeSetCrtc(fd, pDRMEncoder->crtc_id, frameBuffer, 0, 0, &pDRMConnector->connector_id, 1, &pDRMConnector->modes[0]) < 0)
//    {
//        // TODO: cleanup
////        throw std::runtime_error(ErrorMessage::Format(DrmCantSetCrtc,
////                                                      errno));
//        throw std::runtime_error("DrmCantSetCrtc");
//    }
 

    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    int pitch = _drmDumbBuffer.GetPitch();
    
    for (int j = 200; j < 400; j++)
    {
        for (int k = 200; k < 400; k++)
        {
            int offset = pitch * j + k * 4;
            *(uint32_t*)&_frameBufferMap[offset] = (r << 16) | (g << 8) | b;
        }
    }

    
}

FrameBuffer::~FrameBuffer()
{
    munmap(_frameBufferMap, _drmDumbBuffer.GetSize());
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