/* 
 * File:   PrintData.cpp
 * Authors: Richard Greene, Jason Lefly
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

#include <SDL/SDL_image.h>

#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>
#include <Logger.h>
#include <utils.h>

/// Constructor
PrintData::PrintData() : _jobName("")
{
}

/// Destructor
PrintData::~PrintData()
{
}

/// Gets the number of layers to be printed.
int PrintData::GetNumLayers()
{
    glob_t gl;
    size_t numFiles = 0;
    std::string imageFileFilter = SETTINGS.GetString(PRINT_DATA_DIR) + IMAGE_FILE_FILTER;

    if(glob(imageFileFilter.c_str(), GLOB_NOSORT, NULL, &gl) == 0)
      numFiles = gl.gl_pathc;
    
    globfree(&gl);
    
    return numFiles;    
}
    
/// Gets the image  for the given layer
SDL_Surface* PrintData::GetImageForLayer(int layer)
{
    std::ostringstream fileName;
    
    fileName << SETTINGS.GetString(PRINT_DATA_DIR) << "/" << SLICE_IMAGE_PREFIX <<
            "_" << layer << "." << IMAGE_EXTENSION;
    
    SDL_Surface* image = IMG_Load(fileName.str().c_str());
    if(image == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(LoadImageError), fileName.str().c_str());
    }
    return image;
}

/// Move slices from staging directory to print data directory
bool PrintData::MovePrintData()
{
    glob_t gl;
    std::string stagingDir = SETTINGS.GetString(STAGING_DIR);
    std::string printDataDir = SETTINGS.GetString(PRINT_DATA_DIR);
    std::string imageFileFilter = stagingDir + IMAGE_FILE_FILTER;

    glob(imageFileFilter.c_str(), GLOB_NOSORT, NULL, &gl);
    for (size_t i = 0; i < gl.gl_pathc; i++)
    {
        if (!Copy(gl.gl_pathv[i], printDataDir)) return false;
    }

    if (!PurgeDirectory(stagingDir)) return false;
    return true;
}

/// Load settings from settings file in staging directory
bool PrintData::LoadSettings()
{
    std::stringstream buffer;
    std::ifstream settingsFile(SETTINGS.GetString(STAGING_DIR).append(PRINTSETTINGS_FILE).c_str());
    
    if (!settingsFile.is_open()) return false;
    
    buffer << settingsFile.rdbuf();
    
    if (!SETTINGS.LoadFromJSONString(buffer.str())) return false;
    
    return true;
}

/// Validate the contents of the staging directory
bool PrintData::Validate()
{
    std::ostringstream printFile;
    
    // A valid print contains at a minimum the first slice image
    printFile << SETTINGS.GetString(STAGING_DIR) << "/" << SLICE_IMAGE_PREFIX <<
            "_1." << IMAGE_EXTENSION; 
    
    if (!std::ifstream(printFile.str().c_str())) return false;
    
    return true;
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
    
    // Store the file name sans extension as job name
    std::size_t startPos = printFile.find_last_of("/") + 1;
    std::size_t endPos = printFile.find(std::string(".") + PRINT_FILE_EXTENSION);
    _jobName = printFile.substr(startPos, endPos - startPos);
    
    // Remove the print file archive now that it has been extracted
    remove(printFile.c_str());
    
    return true;
}

std::string PrintData::GetJobName()
{
    return _jobName;
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
#ifdef DEBUG
        std::cerr << "could not get handle to archive" << std::endl;
#endif
        return false;
    }

    if (tar_extract_all(tar, rootPathBuf) != 0)
    {
#ifdef DEBUG
        std::cerr << "could not extract archive" << std::endl;
#endif
        retVal = false;
    }
    
    if (tar_close(tar) != 0)
    {
#ifdef DEBUG
        std::cerr << "could not close archive" << std::endl;
#endif
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
#ifdef DEBUG
        std::cerr << "could not open archive with open" << std::endl;
#endif
        return -1;
    }

    if ((oflags & O_CREAT) && fchmod(fd, mode))
    {
#ifdef DEBUG
        std::cerr << "could not do fchmod on archive fd"  << std::endl;
#endif
        return -1;
    }

    gzf = gzdopen(fd, gzoflags);
    if (!gzf)
    {
#ifdef DEBUG
        std::cerr << "could not open archive with gzdopen" << std::endl;
#endif
        errno = ENOMEM;
        return -1;
    }

    return (intptr_t)gzf;
}