//  File:   LayerSettings.h
//  Handles overrides to settings on a per-layer basis, as defined in a CSV file
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
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

#ifndef LAYERSETTINGS_H
#define	LAYERSETTINGS_H

#include <vector>
#include <map>
#include <string>

class LayerSettings {
public:
    virtual ~LayerSettings();
    bool Load(const std::string& layerParams);
    int GetInt(int layer, std::string name);
    double GetDouble(int layer, std::string name);
    void Clear();
    
private:
    std::map<std::string, int> _columns;
    std::map<int, std::vector<double> > _rows;
    std::string Trim(std::string);
    double GetRawValue(int layer, std::string name);

};

// Holds the values of all print settings to use for a single layer 
struct CurrentLayerSettings
{
    // listed here in the order in which they're used
    int PressMicrons;
    int PressMicronsPerSec;
    int PressWaitMS;
    int UnpressMicronsPerSec;
    int ApproachWaitMS;
    double ExposureSec;
    int SeparationRotJerk;
    int SeparationRPM;
    int RotationMilliDegrees;
    int SeparationZJerk;
    int SeparationMicronsPerSec;
    int ZLiftMicrons;
    int ApproachRotJerk;
    int ApproachRPM;
    int ApproachZJerk;
    int ApproachMicronsPerSec;
    int LayerThicknessMicrons;
    
    // these are included to avoid changes while pause & inspect is in progress
    bool CanInspect;
    int InspectionHeightMicrons;
};

#endif    // LAYERSETTINGS_H

