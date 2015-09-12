//  File:   LayerSettings.cpp
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

#include <fstream>
#include <sstream>
#include <limits>
#include <cmath>

#include <LayerSettings.h>
#include <Logger.h>
#include <Settings.h>

using std::string;
using std::map;
using std::vector;
using std::ifstream;
using std::stringstream;

// destructor
LayerSettings::~LayerSettings() 
{
    Clear();
}

// Load per-layer settings overrides from CSVs contained in specified string.
bool LayerSettings::Load(const std::string& layerParams)
{
    Clear();
    
    std::istringstream layerParamsStream(layerParams);
    
    string line;  
    string cell;
    char lineDelim = '\r';  // in order to work with CSV files lacking \n
    char cellDelim = ','; 

    // read the row of headers into a map that tells us which setting 
    // is overridden by each column
    if (std::getline(layerParamsStream, line, lineDelim))
    {  
        stringstream firstLineStream(line);
        
        // skip the first (Layer) column heading        
        int col = -1;

        while(std::getline(firstLineStream, cell, cellDelim))
        {
            string name = Trim(cell);
        
            if (_columns.count(name) < 1)
                _columns[name] = col++;
            else
            {
                LOGGER.HandleError(DuplicateLayerParamsColumn, false, 
                                                                name.c_str());
                Clear();
                return false;
            }
        }
    }
    else
        return false;
    
    // for each row of settings, i.e. for a particular layer
    while(std::getline(layerParamsStream, line, lineDelim))
    {
        int layer;
        double value;
        vector<double> rowData;
        int col = 0;
        stringstream lineStream(line);
        
        // get the layer number
        if (std::getline(lineStream, cell, cellDelim) && 
            (Trim(cell).size() > 0))
        {
            layer = atoi(cell.c_str());
            if (layer < 1)
                continue;   // comment or other invalid row
        }
        else
            continue;
        
        // get the settings, using NaN for any missing ones 
        while(std::getline(lineStream, cell, cellDelim))
            rowData.push_back((Trim(cell).size() > 0)  ? 
                atof(cell.c_str()) : std::numeric_limits<double>::quiet_NaN());

        // check for duplicate layer number
        if (_rows.count(layer) < 1)
            _rows[layer] = rowData;
        else
        {
            LOGGER.HandleError(DuplicateLayerParams, false, NULL, layer);
            Clear();
            return false;
        }
    }
    
    return _rows.size() > 0;
}

// Clear all per-layer settings.
void LayerSettings::Clear()
{
    if (!_rows.empty())
    {
        for (map<int, vector<double> >::iterator it = _rows.begin(); 
                                                 it != _rows.end(); ++it)
        {
            if (!it->second.empty())
                it->second.clear();
        }
    
        _rows.clear();
    }
    _columns.clear();
}

// Trim leading and trailing whitespace from a string.
string LayerSettings::Trim(string input)
{
    const char* whitespace = " \n\r\t";
    
    size_t start = input.find_first_not_of(whitespace);
    size_t end   = input.find_last_not_of (whitespace);

    if ((string::npos == start) || (string::npos == end))
        return "";
    else
        return input.substr(start, end - start + 1);
}

// Get the raw double value contained in this object for the given layer and 
// setting name, if any.  Return NaN if no such value is contained.
double LayerSettings::GetRawValue(int layer, std::string name)
{
    double value = std::numeric_limits<double>::quiet_NaN();
       
    if (_rows.count(layer) > 0 && _columns.count(name) > 0)
    {
        int col = _columns[name];
        value = _rows[layer][col];
    }
    
    return value;  
}

// Get the override for an integer setting, if overridden, else the setting 
// value itself.
int LayerSettings::GetInt(int layer, std::string name)
{
    double value = GetRawValue(layer, name);
    if (!std::isnan(value))
        return (int) value;
    else
        return SETTINGS.GetInt(name);  
}

// Get the override for a double setting, if overridden, else the setting 
// value itself.
double LayerSettings::GetDouble(int layer, std::string name)
{
    double value = GetRawValue(layer, name);
    if (!std::isnan(value))
        return value;
    else
        return SETTINGS.GetDouble(name);  
}