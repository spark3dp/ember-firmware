//  File:   DRM_DumbBuffer.h
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

#include <xf86drm.h>
#include <xf86drmMode.h>

class DRM_Device;
class DRM_Connector;

class DRM_DumbBuffer
{
public:
    DRM_DumbBuffer(const DRM_Device& drmDevice,
                   const DRM_Connector& drmConnector,
                   int width, int height, int bitsPerPixel);
    ~DRM_DumbBuffer();
    uint32_t GetPitch() const;
    uint32_t GetHandle() const;
    uint64_t GetSize() const;
    const drmModeModeInfo& GetModeInfo() const;
    int GetBitsPerPixel() const;
    uint16_t GetWidth() const;
    uint16_t GetHeight() const;

private:
    DRM_DumbBuffer(const DRM_DumbBuffer&);
    DRM_DumbBuffer& operator=(const DRM_DumbBuffer&);
    
    int _drmDeviceFileDescriptor;
    const drmModeModeInfo& _modeInfo;
    int _bitsPerPixel;
    drm_mode_create_dumb _createRequest;
};