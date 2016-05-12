//  File:   DRM_FrameBuffer.h
//  Encapsulates a DRM frame buffer.
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

#include <stdint.h>

class DRM_Device;
class DRM_DumbBuffer;

class DRM_FrameBuffer
{
public:
    DRM_FrameBuffer(const DRM_Device& drmDevice,
                    const DRM_DumbBuffer& drmDumbBuffer, int depth);
    ~DRM_FrameBuffer();
    uint32_t GetId() const;

private:
    DRM_FrameBuffer(const DRM_FrameBuffer&);
    DRM_FrameBuffer& operator=(const DRM_FrameBuffer&);

    int _drmDeviceFileDescriptor;
    uint32_t _id;
};