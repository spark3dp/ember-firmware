//  File:   DRM_Encoder.h
//  Encapsulates a DRM encoder. Encoders help the CRTC to convert data from a
//  frame buffer into the right format for the chosen connector. A CRTC is a
//  controller that manages which data goes to which connector.
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

class DRM_Encoder
{
public:
    DRM_Encoder(const DRM_Device& drmDevice, const DRM_Connector& drmConnector);
    ~DRM_Encoder();
    uint32_t GetCrtcId() const;

private:
    DRM_Encoder(const DRM_Encoder&);
    DRM_Encoder& operator=(const DRM_Encoder&);

    drmModeEncoderPtr _pEncoder;
};