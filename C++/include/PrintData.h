//  File:   PrintData.h
//  Interface to various print data implementations
//  Also holds factory functions
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

#ifndef PRINTDATA_H
#define	PRINTDATA_H

#include <string>

struct SDL_Surface;
class PrintFileStorage;

class PrintData
{
public:
    virtual ~PrintData() {}
    virtual bool Validate() = 0;
    virtual bool GetFileContents(const std::string& fileName,
        std::string& contents) = 0;
    virtual bool Remove() = 0;
    virtual bool Move(const std::string& destination) = 0;
    virtual SDL_Surface* GetImageForLayer(int layer) = 0;
    virtual int GetLayerCount() = 0;
    
    static PrintData* CreateFromNewData(const PrintFileStorage& storage,
        const std::string& dataParentDirectory, const std::string& newName);
    static PrintData* CreateFromExistingData(const std::string& printDataPath);
};

#endif    // PRINTDATA_H

