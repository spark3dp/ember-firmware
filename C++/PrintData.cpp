/* 
 * File:   PrintData.cpp
 * Authors: Richard Greene, Jason Lefley
 * 
 * Handles the data for the 3D model to be printed.
 * 
 * Created on June 4, 2014, 12:45 PM
 */

#include <sys/types.h>
#include <glob.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <libtar.h>
#include <zlib.h>
#include <sstream>
#include <dirent.h>

#include <SDL/SDL_image.h>

#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>
#include <Logger.h>
#include <utils.h>
#include <Shared.h>

/// Constructor
PrintData::PrintData() : 
_fileName("")
{
}

/// Destructor
PrintData::~PrintData()
{
}

/// Gets the number of layers to be printed.
int PrintData::GetNumLayers(const std::string directory)
{
    glob_t gl;
    size_t numFiles = 0;
    std::string imageFileFilter = directory + IMAGE_FILE_FILTER;

    if(glob(imageFileFilter.c_str(), GLOB_NOSORT, NULL, &gl) == 0)
      numFiles = gl.gl_pathc;
    
    globfree(&gl);
    
    return numFiles;    
}
    
/// Gets the image  for the given layer
SDL_Surface* PrintData::GetImageForLayer(int layer)
{
    std::string fileName = GetLayerFileName(SETTINGS.GetString(PRINT_DATA_DIR), 
                                            layer);

    SDL_Surface* image = IMG_Load(fileName.c_str());
    if(image == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(LoadImageError), 
                        fileName.c_str());
    }
    return image;
}

/// Move slices and per-layer settings (if any) from staging directory 
/// to print data directory
bool PrintData::MovePrintData()
{
    glob_t gl;
    bool success = true;
    std::string stagingDir = SETTINGS.GetString(STAGING_DIR);
    std::string printDataDir = SETTINGS.GetString(PRINT_DATA_DIR);
    std::string imageFileFilter = stagingDir + IMAGE_FILE_FILTER;

    glob(imageFileFilter.c_str(), GLOB_NOSORT, NULL, &gl);
    for (size_t i = 0; i < gl.gl_pathc; i++)
    {
        if (!Copy(gl.gl_pathv[i], printDataDir))
        {
           success = false;
           break;
        }
    }
    globfree(&gl);
    
    // copy per layer settings file, if it exists
    std::string layerSettings = stagingDir + PER_LAYER_SETTINGS_FILE;
    if(access(layerSettings.c_str(), F_OK) == 0  && 
       !Copy(layerSettings, printDataDir))
        success = false;
    
    // call fsync to ensure critical data is written to the storage device
    DIR* dir = opendir(printDataDir.c_str());
    if(dir != NULL)
    {
        fsync(dirfd(dir));
        closedir(dir);
    }
        

    if (success)
        return PurgeDirectory(stagingDir);
    else
        return false;
}

/// Load settings from temporary settings file downloaded from the web, if 
/// it exists.  Otherwise use the settings file in staging directory.  
bool PrintData::LoadSettings()
{
    // first restore all print settings to their defaults, in case the new
    // settings don't include all possible settings (e.g. because the print data
    // file was created before some newer settings were defined)
    if(!SETTINGS.RestoreAllPrintSettings())
        return false;
    
    if (access(TEMP_SETTINGS_FILE, F_OK) == 0)
        return SETTINGS.SetFromFile(TEMP_SETTINGS_FILE);
    
    std::string filename = SETTINGS.GetString(STAGING_DIR);
    filename.append(EMBEDDED_PRINT_SETTINGS_FILE);
    return SETTINGS.SetFromFile(filename);                                                                  
}

/// Validate the contents of the staging directory
bool PrintData::Validate(std::string directory)
{    
    int numLayers = GetNumLayers(directory);
    if(numLayers < 1)
        return false;  // a valid print must contain at least one slice image
    
    // check that the slice images are named/numbered as expected
    for(int i = 1; i <= numLayers; i++)
    {
        if(!std::ifstream(GetLayerFileName(directory, i).c_str())) 
            return false;
    }
    
    return true;
}

/// Get the name of the image file in the given directory for the given layer
std::string PrintData::GetLayerFileName(std::string directory, int layer)
{
    std::ostringstream fileName;
    
    fileName << directory << "/" << SLICE_IMAGE_PREFIX << "_" << layer << "." 
             << IMAGE_EXTENSION;  
    
    return fileName.str();
}

/// Extract the first archive in the download directory to the staging directory
bool PrintData::Stage()
{
    std::string stagingDir = SETTINGS.GetString(STAGING_DIR);
    std::string printFileFilter = SETTINGS.GetString(DOWNLOAD_DIR) + PRINT_FILE_FILTER;
    
    // Clear the staging directory
    PurgeDirectory(stagingDir);
    
    // Get an archive in the download folder
    glob_t gl;
    glob(printFileFilter.c_str(), 0, NULL, &gl);
    if (!gl.gl_pathc > 0) return false;
    std::string printFile = gl.gl_pathv[0];
    globfree(&gl);
    
    // Extract the archive
    if (!extractGzipTar(printFile, stagingDir)) return false;
    
    // Store the file name sans temporary path
    std::size_t startPos = printFile.find_last_of("/") + 1;
    _fileName = printFile.substr(startPos);
    
    // Remove the print file archive now that it has been extracted
    remove(printFile.c_str());
    
    return true;
}

std::string PrintData::GetFileName()
{
    return _fileName;
}

bool PrintData::extractGzipTar(std::string archivePath, std::string rootPath)
{
    bool retVal = true;
    char archivePathBuf[archivePath.length()];
    char rootPathBuf[rootPath.length()];
    TAR* tar;
    tartype_t gzType = {
      (openfunc_t)  &PrintData::gzOpenFrontend,
      (closefunc_t) gzclose,
      (readfunc_t)  gzread,
      (writefunc_t) gzwrite
    };
    
    std::strcpy(archivePathBuf, archivePath.c_str());
    std::strcpy(rootPathBuf, rootPath.c_str());
    
    if (tar_open(&tar, archivePathBuf, &gzType, O_RDONLY, 0, 0) == -1)
    {
        std::cerr << "could not get handle to archive" << std::endl;
        return false;
    }

    if (tar_extract_all(tar, rootPathBuf) != 0)
    {
        std::cerr << "could not extract archive" << std::endl;
        retVal = false;
    }
    
    if (tar_close(tar) != 0)
    {
        std::cerr << "could not close archive" << std::endl;
    }

    return retVal;
}

/// Frontend for opening gzip files
/// Taken from libtar.c (demo driver program for libtar)
int PrintData::gzOpenFrontend(char* pathname, int oflags, int mode)
{
    char* gzoflags;
    gzFile gzf;
    int fd;
    
    switch (oflags & O_ACCMODE)
    {
    case O_WRONLY:
        gzoflags = const_cast<char*>("wb");
        break;
    case O_RDONLY:
        gzoflags = const_cast<char*>("rb");
        break;
    default:
    case O_RDWR:
        errno = EINVAL;
        return -1;
    }

    fd = open(pathname, oflags, mode);
    if (fd == -1)
    {
        std::cerr << "could not open archive with open" << std::endl;
        return -1;
    }

    if ((oflags & O_CREAT) && fchmod(fd, mode))
    {
        std::cerr << "could not do fchmod on archive fd"  << std::endl;
        return -1;
    }

    gzf = gzdopen(fd, gzoflags);
    if (!gzf)
    {
        std::cerr << "could not open archive with gzdopen" << std::endl;
        errno = ENOMEM;
        return -1;
    }

    return (intptr_t)gzf;
}

/// remove any existing printable data
bool PrintData::Clear()
{
    return PurgeDirectory(SETTINGS.GetString(PRINT_DATA_DIR));
}

