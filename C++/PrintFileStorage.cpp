/* 
 * File:   PrintFileStorage.cpp
 * Author: Jason Lefley
 * 
 * Provides functionality for finding a print file in a directory.
 * 
 * Created on August 19, 2015, 4:17 PM
 */

#include <glob.h>

#include "PrintFileStorage.h"
#include "Filenames.h"

/// Constructor
/// Looks for a print file in the specified directory
PrintFileStorage::PrintFileStorage(const std::string& directory) :
_filePath(""),
_fileName(""),
_foundTarGz(false),
_foundZip(false)
{
    glob_t glTarGz, glZip;

    std::string printFileFilterTarGz = directory + PRINT_FILE_FILTER_TARGZ;
    std::string printFileFilterZip = directory + PRINT_FILE_FILTER_ZIP;
    
    glob(printFileFilterTarGz.c_str(), 0, NULL, &glTarGz);
    glob(printFileFilterZip.c_str(), 0, NULL, &glZip);

    if (glTarGz.gl_pathc > 0)
    {
        _foundTarGz = true;
        _filePath = glTarGz.gl_pathv[0];

    }

    if (glZip.gl_pathc > 0)
    {
        _foundZip = true;
        _filePath = glZip.gl_pathv[0];

    }
    
    globfree(&glTarGz);
    globfree(&glZip);

    // get the file name if we found a file
    if (_foundZip || _foundTarGz)
        _fileName = _filePath.substr(_filePath.find_last_of("/") + 1);
}

PrintFileStorage::~PrintFileStorage()
{
}
