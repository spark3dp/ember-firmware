//  File:   PrintDataDirectory.h
//  Handles data stored in a directory for the 3D model to be printed
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

#ifndef PRINTDATADIRECTORY_H
#define	PRINTDATADIRECTORY_H

#include <PrintData.h>

class PrintDataDirectory : public PrintData
{
public:
    PrintDataDirectory(const std::string& directoryPath);
    virtual ~PrintDataDirectory();
    bool Validate();
    bool GetFileContents(const std::string& fileName, std::string& contents);
    bool Remove();
    bool Move(const std::string& destination);
    SDL_Surface* GetImageForLayer(int layer);
    int GetLayerCount();

private:
    std::string GetLayerFileName(int layer);

private:
    std::string _directoryPath; // the directory containing the print data
};

#endif    // PRINTDATADIRECTORY_H

