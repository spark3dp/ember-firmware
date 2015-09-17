//  File:   PrintFileStorage.h
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

#ifndef PRINTFILESTORAGE_H
#define	PRINTFILESTORAGE_H

#include <string>

class PrintFileStorage
{
public:
    PrintFileStorage(const std::string& directory);
    ~PrintFileStorage();
    std::string GetFileName() const { return _fileName; }
    std::string GetFilePath() const { return _filePath; }
    bool HasZip() const { return _foundZip; }
    bool HasTarGz() const { return _foundTarGz; }
    bool HasOneFile() const { return _foundCount == 1; }

private:
    std::string _filePath;
    std::string _fileName;
    bool _foundTarGz;
    bool _foundZip;
    int _foundCount;
};

#endif    // PRINTFILESTORAGE_H

