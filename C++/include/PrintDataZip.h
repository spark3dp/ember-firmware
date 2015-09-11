//  File:   PrintDataZip.h
//  Handles data stored in a zip file for the 3D model to be printed
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

#ifndef PRINTDATAZIP_H
#define	PRINTDATAZIP_H

#include <zpp.h>

#include <PrintData.h>

class PrintDataZip : public PrintData
{
public:
    PrintDataZip(const std::string& filePath);
    virtual ~PrintDataZip();
    bool Validate();
    bool GetFileContents(const std::string& fileName, std::string& contents);
    bool Remove();
    bool Move(const std::string& destination);
    SDL_Surface* GetImageForLayer(int layer);
    int GetLayerCount();

    static void Initialize();

private:
    std::string GetLayerFileName(int layer);

private:
    std::string _filePath;     // the path to the zip file backing this instance
    zppZipArchive _zipArchive; // zpp zip archive wrapper
};

#endif    // PRINTDATAZIP_H

