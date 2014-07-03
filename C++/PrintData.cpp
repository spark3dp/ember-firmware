/* 
 * File:   PrintData.cpp
 * Author: Richard Greene
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
#include <MessageStrings.h>
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
    char fileName[PATH_MAX];
    
    sprintf(fileName, "%s/%s_%d.%s", SETTINGS.GetString(PRINT_DATA_DIR).c_str(),
            SLICE_IMAGE_PREFIX, layer, IMAGE_EXTENSION);

    SDL_Surface* image = IMG_Load(fileName);
    if(image == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, LOAD_IMAGE_ERROR, fileName);
    }
    return image;
}

/// Validate the contents of the staging directory
bool PrintData::Validate()
{
    // A valid print contains at a minimum one slice image named slice_1.png
    std::string firstSlice = SETTINGS.GetString(STAGING_DIR) + "/slice_1.png";
    if (!std::ifstream(firstSlice.c_str())) return false;
    
    return true;
}

/// Extract the first archive in the download directory to the staging directory
bool PrintData::Stage()
{
    std::string stagingDirSetting = SETTINGS.GetString(STAGING_DIR);
    char stagingDir[stagingDirSetting.length()];
    strcpy(stagingDir, stagingDirSetting.c_str());
    std::string printFileFilter = SETTINGS.GetString(DOWNLOAD_DIR) + PRINT_FILE_FILTER;
    
    // Clear the staging directory
    PurgeDirectory(stagingDir);
    
    // Get an archive in the download folder
    glob_t gl;
    glob(printFileFilter.c_str(), 0, NULL, &gl);
    //if (!gl.gl_pathc > 0) return false;
    char printFile[strlen(gl.gl_pathv[0])];
    strcpy(printFile, gl.gl_pathv[0]);
    globfree(&gl);
    
    std::cout << "Found a print file: " << printFile << std::endl;
    
    // Extract the archive
    TAR* tar;
    tartype_t gzType = {
      (openfunc_t)  &PrintData::gzOpenFrontend,
      (closefunc_t) gzclose,
      (readfunc_t)  gzread,
      (writefunc_t) gzwrite
    };
    
    if (tar_open(&tar, printFile, &gzType, O_RDONLY, 0, 0) == -1)
    {
        std::cout << "Could not get handle to archive" << std::endl;
    }

    if (tar_extract_all(tar, stagingDir) != 0)
    {
        std::cout << "Could not extract archive" << std::endl;
    }
    
    if (tar_close(tar) != 0)
    {
        std::cout << "Could not close archive" << std::endl;
    }
    
    // Remove the print file archive now that it has been extracted
    remove(printFile);
    
    return true;
}

std::string PrintData::GetJobName()
{
    return _jobName;
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
            gzoflags = "wb";
            break;
    case O_RDONLY:
            gzoflags = "rb";
            break;
    default:
    case O_RDWR:
            errno = EINVAL;
            return -1;
    }

    fd = open(pathname, oflags, mode);
    if (fd == -1)
            return -1;

    if ((oflags & O_CREAT) && fchmod(fd, mode))
            return -1;

    gzf = gzdopen(fd, gzoflags);
    if (!gzf)
    {
            errno = ENOMEM;
            return -1;
    }

    return (int)gzf;
}