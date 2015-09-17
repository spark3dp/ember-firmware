//  File:   PrintDataZip.cpp
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

#include <SDL/SDL_image.h>
#include <sstream>

#include <Logger.h>
#include <PrintDataZip.h>
#include <Filenames.h>

// Constructor
// filePath is the path to the zip file that backs this instance
PrintDataZip::PrintDataZip(const std::string& filePath) :
_filePath(filePath),
_zipArchive(zppZipArchive(filePath, std::ios_base::in, false))
{
}

PrintDataZip::~PrintDataZip()
{
}

// Gets the image for the given layer
SDL_Surface* PrintDataZip::GetImageForLayer(int layer)
{
    // create a stream to access zip file contents
    izppstream layerFile;

    std::string fileName = GetLayerFileName(layer);
    // assume the client previously validated the data and the specified layer 
    // file opens successfully
    layerFile.open(fileName, &_zipArchive);

    // read file into buffer
    std::stringstream ss;
    ss << layerFile.rdbuf();
    std::string buffer = ss.str();

    // load as image
    SDL_Surface* image = NULL;
    SDL_RWops* rwop = SDL_RWFromConstMem(buffer.data(), buffer.size());

    if (rwop != NULL)
    {
        image = IMG_LoadPNG_RW(rwop);
        SDL_RWclose(rwop);
    }

    if (image == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(LoadImageError), 
                                                            fileName.c_str());
    }
    
    return image;
}

// Get the number of layers contained in the print data
int PrintDataZip::GetLayerCount()
{
    int sliceCount = 0;
    const zppFileMap& fileMap = _zipArchive.getFileMap();

    for (zppFileMap::const_iterator it = fileMap.begin(); 
                                    it != fileMap.end(); it++)
    {
        size_t idx = it->first.rfind('.');
        if  (idx != std::string::npos &&
                it->first.substr(idx + 1) == SLICE_IMAGE_EXTENSION &&
                it->first.substr(0, 6) == SLICE_IMAGE_PREFIX)
        {
            sliceCount++;
        }
    }

    return sliceCount;
}

// If the print data contains the specified file, read contents into specified 
// string and return true.  Otherwise, return false.
bool PrintDataZip::GetFileContents(const std::string& fileName, 
                                   std::string& contents)
{
    // create a stream to access zip file contents
    izppstream settingsFile;

    // open the file
    settingsFile.open(fileName.c_str(), &_zipArchive);
    if (settingsFile.good())
    {
        // update specified string with contents
        std::stringstream buffer;
        buffer << settingsFile.rdbuf();
        contents = buffer.str();
        return true;
    }
    else
        return false;
}

// Move the print data zip file into destination
bool PrintDataZip::Move(const std::string& destination)
{
    // figure out the file name without directory
    // this operation keeps the slash preceeding the file name
    std::string fileName(_filePath);
    fileName.erase(0, fileName.find_last_of("/"));
    
    std::string newFilePath = destination + fileName;

    if (rename(_filePath.c_str(), newFilePath.c_str()) == 0)
    {
        _filePath = newFilePath;
        return true;
    }

    return false;
}

// Remove the print data zip file
bool PrintDataZip::Remove()
{
    return remove(_filePath.c_str()) == 0;
}

// Validate the print data
bool PrintDataZip::Validate()
{
    int layerCount = GetLayerCount();

    if (layerCount < 1)
        return false;  // a valid print must contain at least one slice image

    // create a stream to access zip file contents
    izppstream layerFile;
    
    // check that the slice images are named/numbered as expected
    for(int i = 1; i <= layerCount; i++)
    {
        layerFile.open(GetLayerFileName(i), &_zipArchive);
        if (!layerFile.good())
            return false;
        layerFile.close();
    }

    return true;
}

// Get the name of the image file for the given layer
std::string PrintDataZip::GetLayerFileName(int layer)
{
    std::ostringstream fileName;

    fileName << SLICE_IMAGE_PREFIX << layer << "." << SLICE_IMAGE_EXTENSION;

    return fileName.str();
}

// Initialize zpp library settings
void PrintDataZip::Initialize()
{
    // Don't parse zip file attributes when loading archive
    zppZipArchive::setParseAttrFlag(false);
}
