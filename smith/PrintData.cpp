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


#include <PrintData.h>
#include <Filenames.h>
#include <Settings.h>

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
    
char fileName[PATH_MAX];

/// Gets the image file name for the given layer
char* PrintData::GetFilenameForLayer(int layer)
{
    // TODO get name from setting, but just hard code it for now
    const char* jobName = Settings::GetString("JobName");
    sprintf(fileName, "%s/%s_%04d.%s", 
                       IMAGE_FOLDER, jobName, layer, IMAGE_EXTENSION);
    return fileName;
}
    