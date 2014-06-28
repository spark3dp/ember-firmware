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
//#include <libtar.h>

#include <SDL/SDL_image.h>

#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>
#include <MessageStrings.h>
#include <Logger.h>

/// Constructor
PrintData::PrintData() {
}

/// Destructor
PrintData::~PrintData() {
}

/// Gets the number of layers to be printed.
int PrintData::GetNumLayers()
{
    glob_t gl;
    size_t numFiles = 0;

    if(glob(IMAGE_FILE_FILTER, GLOB_NOSORT, NULL, &gl) == 0)
      numFiles = gl.gl_pathc;
    
    globfree(&gl);
    
    return numFiles;    
}
    
/// Gets the image  for the given layer
SDL_Surface* PrintData::GetImageForLayer(int layer)
{
    char fileName[PATH_MAX];
    
    std::string jobName = SETTINGS.GetString(JOB_NAME_SETTING);
    sprintf(fileName, "%s/%s_%04d.%s", 
                       IMAGE_FOLDER, jobName.c_str(), layer, IMAGE_EXTENSION);

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
    // A valid print contains at a minimum one slice image named slice_0001.png
    std::string firstSlice = std::string(STAGING_FOLDER) + "/slice_0001.png";
    if (!std::ifstream(firstSlice.c_str())) return false;
    
    return true;
}

/// Extract the first archive in the download folder into the staging folder
bool PrintData::Stage()
{
    // Get an archive in the download folder
    glob_t gl;
    std::string printFile;
    
    glob(PRINT_FILE_FILTER, NULL, NULL, &gl);
    
    //if (!gl.gl_pathc > 0) return false;
    
    printFile = gl.gl_pathv[0];
    
    std::cout << "Found a print file: " << printFile << std::endl;
    
    //TAR* tar;
    
    //std::cout << "Opening tarfile" << std::endl;
    
    //tar_open(&tar, printFile.c_str(), , NULL, O_RDONLY, false, )
    
    globfree(&gl);
    
    return true;
}