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
#include <sys/stat.h>

#include <PrintData.h>
#include <PrintDataDirectory.h>
#include <PrintDataZip.h>
#include <Filenames.h>
#include <utils.h>
#include <TarGzFile.h>

/// Look for a print file in the specified downloadDirectory and return an appropriate PrintData
/// instance, placing the print data in the specified dataDirectory
PrintData* PrintData::CreateFromNewData(const std::string& downloadDirectory, const std::string& dataParentDirectory)
{
    std::string printFileFilterTarGz = downloadDirectory + PRINT_FILE_FILTER_TARGZ;
    std::string printFileFilterZip = downloadDirectory + PRINT_FILE_FILTER_ZIP;
    
    // Clear the specified data parent directory
    // Since this method uses the name of the file as the directory to extract to, directory name
    // collisions may occur if a print file with the same name is loaded twice
    PurgeDirectory(dataParentDirectory);
    
    // Look for print files in specified download directory
    glob_t glTarGz, glZip;
    bool foundTarGz = false, foundZip = false;
    std::string printFile;
    
    glob(printFileFilterTarGz.c_str(), 0, NULL, &glTarGz);
    glob(printFileFilterZip.c_str(), 0, NULL, &glZip);

    // Inspect results of glob, considering tar.gz first
    if (glTarGz.gl_pathc > 0)
    {
        foundTarGz = true;
        printFile = glTarGz.gl_pathv[0];

    }

    if (glZip.gl_pathc > 0)
    {
        foundZip = true;
        printFile = glZip.gl_pathv[0];

    }
    
    globfree(&glTarGz);
    globfree(&glZip);
    
    // Get the file name
    std::size_t startPos = printFile.find_last_of("/") + 1;
    std::string fileName = printFile.substr(startPos);

    // Create a destination path for the print data
    // For a tar.gz, the archive is extracted into a directory at this path
    // For a zip, the archive is renamed to this path
    std::string printDataDestination = dataParentDirectory + "/" + fileName;

    if (foundTarGz)
    {
        // Make a directory to extract to
        mkdir(printDataDestination.c_str(), 0755);
        
        // Extract the archive
        bool extractSuccessful = TarGzFile::Extract(printFile, printDataDestination);

        // Remove the print file regardless of extraction success
        remove(printFile.c_str());

        if (!extractSuccessful)
        {
            // Cleanup if extract failed
            rmdir(printDataDestination.c_str());
            return NULL;
        }
        
        return new PrintDataDirectory(fileName, printDataDestination);
    }
    else if (foundZip)
    {
        // Move the zip file to the specified parent directory
        rename(printFile.c_str(), printDataDestination.c_str());
        PrintDataZip::Initialize();
        try
        {
            return new PrintDataZip(fileName, printDataDestination);
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
