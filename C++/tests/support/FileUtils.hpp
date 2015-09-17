//  File:   FileUtils.hpp
//  File utiloitoes, for use in automated tests
//  Exposed as a pollable resource
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

#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>

#include <utils.h>

void Touch(std::string path)
{
    std::ofstream slice(path.c_str());
    slice.close();
}

void RemoveDir(std::string path)
{
    PurgeDirectory(path);
    remove(path.c_str());
}

std::string CreateTempDir()
{
    std::string tempDir;
    char tempDirTemplate[] = "/tmp/XXXXXX";
    tempDir = mkdtemp(tempDirTemplate);
    return tempDir;
}

/// Return the number of entries contained in the specified directory of specified type
int GetEntryCount(const std::string& directory, unsigned char type)
{
    int count = 0;
    DIR* dir = opendir(directory.c_str());
    struct dirent* entry = readdir(dir);

    while (entry != NULL)
    {
        if (entry->d_type == type && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
            count++;
        entry = readdir(dir);
    }

    closedir(dir);
    return count;
}
