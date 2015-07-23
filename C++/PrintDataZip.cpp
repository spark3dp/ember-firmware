/* 
 * File:   PrintDataZip.cpp
 * Author: Jason Lefley
 * 
 * Handles data stored in a zip file for the 3D model to be printed.
 * 
 * Created on July 16, 2015, 5:49 PM
 */

#include <PrintDataZip.h>
#include <Filenames.h>

/// Constructor
/// fileName is the name of the zip file that backs this instance
/// filePath is the path to the zip file that backs this instance
PrintDataZip::PrintDataZip(const std::string& fileName, const std::string& filePath) :
_fileName(fileName),
_zipArchive(zppZipArchive(filePath, std::ios_base::in, false))
{
}

PrintDataZip::~PrintDataZip()
{
}

std::string PrintDataZip::GetFileName()
{
    return _fileName;
}

SDL_Surface* PrintDataZip::GetImageForLayer(int layer)
{
}

// Get the number of layers contained in the print data
int PrintDataZip::GetLayerCount()
{
    int sliceCount = 0;
    const zppFileMap& fileMap = _zipArchive.getFileMap();

    for (zppFileMap::const_iterator it = fileMap.begin(); it != fileMap.end(); it++)
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

bool PrintDataZip::GetSettings(std::string& settings)
{
    return false;
}

bool PrintDataZip::Move(const std::string& destination)
{
    return false;
}

bool PrintDataZip::Remove()
{
    return false;
}

bool PrintDataZip::Validate()
{
    return false;
}

/// Initialize zpp library settings
void PrintDataZip::Initialize()
{
    // Don't parse zip file attributes when loading archive
    zppZipArchive::setParseAttrFlag(false);
}
