//  File:   PrintData.cpp
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

#include <sys/stat.h>
#include <string.h>

#include <PrintData.h>
#include <PrintDataDirectory.h>
#include <PrintDataZip.h>
#include <utils.h>
#include <TarGzFile.h>
#include "PrintFileStorage.h"

// Use the specified storage object to find a print file and return an
// appropriate PrintData instance, placing the print data in the specified
// dataParentDirectory. The print data is renamed to or placed in a directory
// named according to specified newName.
PrintData* PrintData::CreateFromNewData(const PrintFileStorage& storage,
        const std::string& dataParentDirectory, const std::string& newName)
{
    // avoid naming collisions by clearing the specified data parent directory
    PurgeDirectory(dataParentDirectory);

    // create a destination path for the print data
    // for a tar.gz, the archive is extracted into a directory at this path
    // for a zip, the archive is renamed to this path
    std::string printDataDestination = dataParentDirectory + "/" + newName;

    if (storage.HasTarGz())
    {
        // make a directory to extract to
        mkdir(printDataDestination.c_str(), 0755);
        
        // extract the archive
        bool extractSuccessful = TarGzFile::Extract(storage.GetFilePath(),
                printDataDestination);

        // remove the print file regardless of extraction success
        remove(storage.GetFilePath().c_str());

        if (!extractSuccessful)
        {
            // cleanup if extract failed
            PurgeDirectory(printDataDestination);
            rmdir(printDataDestination.c_str());
            return NULL;
        }
        
        return new PrintDataDirectory(printDataDestination);
    }
    else if (storage.HasZip())
    {
        // move the zip file to the specified parent directory
        rename(storage.GetFilePath().c_str(), printDataDestination.c_str());
        PrintDataZip::Initialize();
        try
        {
            return new PrintDataZip(printDataDestination);
        }
        catch (const zppError& e)
        {
            // not a valid zip file
            // remove unusable file
            remove(printDataDestination.c_str());
            return NULL;
        }
    }
    else
        // did not find a recognized file
        return NULL;
}

// Look for a file or directory named specified by printDataPath.
// Return a pointer to an appropriate PrintData instance depending on if the
// function found a zip file or directory.
PrintData* PrintData::CreateFromExistingData(const std::string& printDataPath)
{
    struct stat statBuffer;
    memset(&statBuffer, 0, sizeof(struct stat));
    stat(printDataPath.c_str(), &statBuffer);

    // check if printDataPath is a directory
    // if not, check if it is a file, if it is assume zip file
    if (S_ISDIR(statBuffer.st_mode))
    {
        // directory
        return new PrintDataDirectory(printDataPath);
    }
    else if (S_ISREG(statBuffer.st_mode))
    {
        // zip file
        try
        {
            return new PrintDataZip(printDataPath);
        }
        catch (const zppError& e)
        {
            // not a valid zip file
            return NULL;
        }
    }
    else
    {
        // printDataPath does not point to a regular file or directory
        return NULL;
    }
}
