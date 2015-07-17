/* 
 * File:   PrintData.cpp
 * Author: Jason Lefley
 * 
 * Interface to various print data implementations
 * Also holds factory functions
 *
 * Created on July 16, 2015, 3:19 PM
 */

#include <glob.h>
#include <sstream>
#include <sys/stat.h>

#include <PrintData.h>
#include <PrintDataDirectory.h>
#include <Filenames.h>
#include <utils.h>
#include <TarGzFile.h>

/// Look for a print file in the specified downloadDirectory and return an appropriate PrintData
/// instance, placing the print data in the specified dataDirectory
PrintData* PrintData::CreateFromNewData(const std::string& downloadDirectory, const std::string& dataParentDirectory)
{
    std::string printFileFilter = downloadDirectory + PRINT_FILE_FILTER;
    
    // Clear the specified data parent directory
    // Since this method uses the name of the file as the directory to extract to, directory name
    // collisions may occur if a print file with the same name is loaded twice
    PurgeDirectory(dataParentDirectory);
    
    // Get an archive in the download directory
    glob_t gl;
    glob(printFileFilter.c_str(), 0, NULL, &gl);
    if (!gl.gl_pathc > 0) return NULL;
    std::string printFile = gl.gl_pathv[0];
    globfree(&gl);
    
    // Get the file name
    std::size_t startPos = printFile.find_last_of("/") + 1;
    std::string fileName = printFile.substr(startPos);

    // Make a directory to extract to
    std::ostringstream ss;
    ss << dataParentDirectory << "/" << fileName;
    std::string dataDirectory = ss.str();
    mkdir(dataDirectory.c_str(), 0755);
    
    // Extract the archive
    bool extractSuccessful = TarGzFile::Extract(printFile, dataDirectory);
    
    // Remove the print file regardless of extraction success
    remove(printFile.c_str());

    if (!extractSuccessful)
    {
        // Cleanup if extract failed
        rmdir(dataDirectory.c_str());
        return NULL;
    }
    
    return new PrintDataDirectory(fileName, dataDirectory);
}

/// Create an appropriate instance of PrintData depending on specified fileName
PrintData* PrintData::CreateFromExistingData(const std::string& fileName, const std::string& dataParentDirectory)
{
    if (fileName.empty())
        // Print data not present
        return NULL;

    std::ostringstream ss;
    ss << dataParentDirectory << "/" << fileName;
    std::string dataDirectory = ss.str();
    
    struct stat buffer;
    stat(dataDirectory.c_str(), &buffer);
    
    if (!S_ISDIR(buffer.st_mode))
        // PrintDataDirectory must encapsulate a directory
        return NULL;
    
    return new PrintDataDirectory(fileName, dataDirectory);
}
