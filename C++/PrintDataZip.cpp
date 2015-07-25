/* 
 * File:   PrintDataZip.cpp
 * Author: Jason Lefley
 * 
 * Handles data stored in a zip file for the 3D model to be printed.
 * 
 * Created on July 16, 2015, 5:49 PM
 */

#include <SDL/SDL_image.h>
#include <sstream>

#include <Logger.h>
#include <PrintDataZip.h>
#include <Filenames.h>

/// Constructor
/// fileName is the name of the zip file that backs this instance
/// filePath is the path to the zip file that backs this instance
PrintDataZip::PrintDataZip(const std::string& fileName, const std::string& filePath) :
_fileName(fileName),
_filePath(filePath),
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

/// Gets the image for the given layer
SDL_Surface* PrintDataZip::GetImageForLayer(int layer)
{
    // create a stream to access zip file contents
    izppstream layerFile;

    std::string fileName = GetLayerFileName(layer);
    // assume the client previously validated the data and the specified layer file opens successfully
    layerFile.open(fileName, &_zipArchive);

    // read file into buffer
    std::stringstream ss;
    ss << layerFile.rdbuf();
    std::string buffer = ss.str();

    std::cout << "slice image buffer.size(): " << buffer.size() << std::endl;
    
    // load as image
    SDL_Surface* image = NULL;
    SDL_RWops* rwop = SDL_RWFromConstMem(buffer.data(), buffer.size());

    if (rwop != NULL)
    {
        image = IMG_LoadPNG_RW(rwop);
        SDL_RWclose(rwop);
    }

    if(image == NULL)
    {
        std::ostringstream ss;
        ss << fileName << " (in " << _fileName << ")";
        std::string errorDetail = ss.str();
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(LoadImageError), errorDetail.c_str());
    }
    
    return image;
}

/// Get the number of layers contained in the print data
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

/// If the print data contains the specified file, read contents into specified string and return true
/// Otherwise, return false
bool PrintDataZip::GetFileContents(const std::string& fileName, std::string& settings)
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
        settings = buffer.str();
        return true;
    }
    else
        return false;
}

/// Move the print data zip file into destination
bool PrintDataZip::Move(const std::string& destination)
{
    std::string newFilePath = destination + "/" + _fileName;

    if (rename(_filePath.c_str(), newFilePath.c_str()) == 0)
    {
        _filePath = newFilePath;
        return true;
    }

    return false;
}

/// Remove the print data zip file
bool PrintDataZip::Remove()
{
    return remove(_filePath.c_str()) == 0;
}

/// Validate the print data
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

/// Get the name of the image file for the given layer
std::string PrintDataZip::GetLayerFileName(int layer)
{
    std::ostringstream fileName;

    fileName << SLICE_IMAGE_PREFIX << layer << "." << SLICE_IMAGE_EXTENSION;

    return fileName.str();
}

/// Initialize zpp library settings
void PrintDataZip::Initialize()
{
    // Don't parse zip file attributes when loading archive
    zppZipArchive::setParseAttrFlag(false);
}
