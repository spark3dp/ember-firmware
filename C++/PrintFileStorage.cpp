//  File:   PrintFileStorage.cpp
//  Provides functionality for finding a print file in a directory
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

#include <glob.h>

#include "PrintFileStorage.h"
#include "Filenames.h"

// Constructor
// Looks for a print file in the specified directory
PrintFileStorage::PrintFileStorage(const std::string& directory) :
_filePath(""),
_fileName(""),
_foundTarGz(false),
_foundZip(false),
_foundCount(0)
{
    glob_t glTarGz, glZip;

    std::string printFileFilterTarGz = directory + PRINT_FILE_FILTER_TARGZ;
    std::string printFileFilterZip = directory + PRINT_FILE_FILTER_ZIP;
    
    glob(printFileFilterTarGz.c_str(), 0, NULL, &glTarGz);
    glob(printFileFilterZip.c_str(), 0, NULL, &glZip);

    if (glTarGz.gl_pathc > 0)
    {
        _foundTarGz = true;
        _filePath = glTarGz.gl_pathv[0];
        _foundCount += glTarGz.gl_pathc;

    }

    if (glZip.gl_pathc > 0)
    {
        _foundZip = true;
        _filePath = glZip.gl_pathv[0];
        _foundCount += glZip.gl_pathc;

    }
    
    globfree(&glTarGz);
    globfree(&glZip);

    // get the file name if we found a file
    if (_foundZip || _foundTarGz)
        _fileName = _filePath.substr(_filePath.find_last_of("/") + 1);
}

PrintFileStorage::~PrintFileStorage()
{
}
