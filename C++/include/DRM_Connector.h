//  File:   DRM_Connector.h
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

#include <xf86drm.h>
#include <xf86drmMode.h>

class DRM_Device;

class DRM_Connector
{
public:
    DRM_Connector(const DRM_Device& drmDevice, uint32_t id);
    ~DRM_Connector();
    bool IsConnected() const;
    uint32_t GetEncoderId() const;
    uint32_t GetId() const;
    const drmModeModeInfo& GetModeInfo(int width, int height) const;

private:
    DRM_Connector(const DRM_Connector&);
    DRM_Connector& operator=(const DRM_Connector&);

    drmModeConnectorPtr _pConnector;
};