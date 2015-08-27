/* 
 * File:   PrintDataDirectory.cpp
 * Authors: Richard Greene, Jason Lefley
 * 
 * Handles data stored in a directory for the 3D model to be printed
 * 
 * Created on June 4, 2014, 12:45 PM
 */

#include <SDL/SDL_image.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <glob.h>

#include <PrintDataDirectory.h>
#include <Logger.h>
#include <Filenames.h>
#include <utils.h>

/// Constructor
PrintDataDirectory::PrintDataDirectory(const std::string& directoryPath) :
_directoryPath(directoryPath)
{
}

/// Destructor
PrintDataDirectory::~PrintDataDirectory()
{
}

/// Gets the image for the given layer
SDL_Surface* PrintDataDirectory::GetImageForLayer(int layer)
{
    std::string fileName = GetLayerFileName(layer);

    SDL_Surface* image = IMG_Load(fileName.c_str());
    if(image == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(LoadImageError), 
                        fileName.c_str());
    }
    return image;
}

/// If the print data contains the specified file, read contents into specified string and return true
/// Otherwise, return false
bool PrintDataDirectory::GetFileContents(const std::string& fileName, std::string& contents)
{
    std::string path = _directoryPath + "/" + fileName;
    std::ifstream settingsFile(path.c_str());
    if (settingsFile.good())
    {
        std::stringstream buffer;
        buffer << settingsFile.rdbuf();
        contents = buffer.str();
        return true;
    }
    else
        return false;
}

/// Validate the print data
bool PrintDataDirectory::Validate()
{
    int numLayers = GetLayerCount();

    if(numLayers < 1)
        return false;  // a valid print must contain at least one slice image
    
    // check that the slice images are named/numbered as expected
    for(int i = 1; i <= numLayers; i++)
        if(!std::ifstream(GetLayerFileName(i).c_str())) 
            return false;
    
    return true;
}

/// Remove the print data and the directory containing it
bool PrintDataDirectory::Remove()
{
    return PurgeDirectory(_directoryPath) && (rmdir(_directoryPath.c_str()) == 0);
}

/// Move the directory containing the print data into destination
bool PrintDataDirectory::Move(const std::string& destination)
{
    // figure out the file name without directory
    // this operation keeps the slash preceeding the file name
    std::string fileName(_directoryPath);
    fileName.erase(0, fileName.find_last_of("/"));

    std::string newDataDirectory = destination + fileName;

    if (rename(_directoryPath.c_str(), newDataDirectory.c_str()) == 0)
    {
        // call fsync to ensure critical data is written to the storage device
        DIR* dir = opendir(newDataDirectory.c_str());
        if(dir != NULL)
        {
            fsync(dirfd(dir));
            closedir(dir);
        }
    
        _directoryPath = newDataDirectory;
        return true;
    }

    return false;
}

// Get the number of layers contained in the print data
int PrintDataDirectory::GetLayerCount()
{
    glob_t gl;
    size_t numFiles = 0;
    std::string imageFileFilter = _directoryPath + SLICE_IMAGE_FILE_FILTER;

    if(glob(imageFileFilter.c_str(), GLOB_NOSORT, NULL, &gl) == 0)
      numFiles = gl.gl_pathc;
    
    globfree(&gl);
    
    return numFiles;
}

/// Get the name of the image file for the given layer
std::string PrintDataDirectory::GetLayerFileName(int layer)
{
    std::ostringstream fileName;
    
    fileName << _directoryPath << "/" << SLICE_IMAGE_PREFIX << layer << "." 
             << SLICE_IMAGE_EXTENSION;

    return fileName.str();
}
