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