/* 
 * File:   PrintData.cpp
 * Author: Jason Lefley
 * 
 * Interface to various print data implementations
 * Also holds factory functions
 *
 * Created on July 16, 2015, 3:19 PM
 */

#include <sys/stat.h>

#include <PrintData.h>
#include <PrintDataDirectory.h>
#include <PrintDataZip.h>
#include <utils.h>
#include <TarGzFile.h>
#include "PrintFileStorage.h"

/// Look for a print file in the specified downloadDirectory and return an appropriate PrintData
/// instance, placing the print data in the specified dataDirectory
PrintData* PrintData::CreateFromNewData(const std::string& downloadDirectory, const std::string& dataParentDirectory)
{
    // Clear the specified data parent directory
    // Since this method uses the name of the file as the directory to extract to, directory name
    // collisions may occur if a print file with the same name is loaded twice
    PurgeDirectory(dataParentDirectory);

    PrintFileStorage storage(downloadDirectory);
   
    // Create a destination path for the print data
    // For a tar.gz, the archive is extracted into a directory at this path
    // For a zip, the archive is renamed to this path
    std::string printDataDestination = dataParentDirectory + "/" +
            storage.GetFileName();

    if (storage.HasTarGz())
    {
        // Make a directory to extract to
        mkdir(printDataDestination.c_str(), 0755);
        
        // Extract the archive
        bool extractSuccessful = TarGzFile::Extract(storage.GetFilePath(),
                printDataDestination);

        // Remove the print file regardless of extraction success
        remove(storage.GetFilePath().c_str());

        if (!extractSuccessful)
        {
            // Cleanup if extract failed
            rmdir(printDataDestination.c_str());
            return NULL;
        }
        
        return new PrintDataDirectory(storage.GetFileName(), printDataDestination);
    }
    else if (storage.HasZip())
    {
        // Move the zip file to the specified parent directory
        rename(storage.GetFilePath().c_str(), printDataDestination.c_str());
        PrintDataZip::Initialize();
        try
        {
            return new PrintDataZip(storage.GetFileName(), printDataDestination);
        }
        catch (const zppError& e)
        {
            // Not a valid zip file
            // Remove unusable file
            remove(printDataDestination.c_str());
            return NULL;
        }
    }
    else
        // Did not find a recognized file
        return NULL;
}

/// Create an appropriate instance of PrintData depending on specified fileName
PrintData* PrintData::CreateFromExistingData(const std::string& fileName, const std::string& dataParentDirectory)
{
    if (fileName.empty())
        // Print data not present
        return NULL;

    std::string printDataPath = dataParentDirectory + "/" + fileName;

    if (fileName.substr(fileName.find_last_of(".") + 1) == "zip")
    {
        // Zip file
        try
        {
            return new PrintDataZip(fileName, printDataPath);
        }
        catch (const zppError& e)
        {
            // Not a valid zip file
            return NULL;
        }
    }
    else
    {
        // Otherwise check for directory
        struct stat buffer;
        stat(printDataPath.c_str(), &buffer);
        if (!S_ISDIR(buffer.st_mode))
            // PrintDataDirectory must encapsulate a directory
            return NULL;
        
        return new PrintDataDirectory(fileName, printDataPath);
    }
}
