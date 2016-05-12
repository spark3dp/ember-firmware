//  File:   FrameBuffer.h
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

#ifndef FRAMEBUFFER_H
#define	FRAMEBUFFER_H

#include <vector>

#include "IFrameBuffer.h"
#include "DRM_Device.h"
#include "DRM_Resources.h"
#include "DRM_Connector.h"
#include "DRM_Encoder.h"
#include "DRM_DumbBuffer.h"
#include "DRM_FrameBuffer.h"

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
    uint8_t* _pFrameBufferMap;
    std::vector<uint8_t> _image;
};


#endif  // FRAMEBUFFER_H

