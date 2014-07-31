/* 
 * File:   utils.cpp
 * Author: Richard Greene
 * 
 * Some handy utilities
 *
 * Created on April 17, 2014, 3:16 PM
 */

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <Filenames.h>
#include <Version.h>
#include <Logger.h>
#include <Error.h>

/// Get the current time in millliseconds
long GetMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

long startTime = 0;

/// Start the stopwatch timer
void StartStopwatch()
{
    startTime = GetMillis();    
}

/// Stop the stopwatch and return its time in millliseconds
long StopStopwatch()
{
    return GetMillis() - startTime;
}

/// Convert the given string to upper case, and terminate it at whitespace
char* CmdToUpper(char* cmd)
{
    char* p = cmd;
    while(*p != 0)
    {
        *p++ = toupper(*p);
        if(isspace(*p))
            *p = 0;
    }
    return cmd;
}


/// Replace all instances of the oldVal in str with the newVal
std::string Replace(std::string str, const char* oldVal, const char* newVal)
{
    int pos = 0; 
    while((pos = str.find(oldVal)) != std::string::npos)
         str.replace(pos, 1, newVal); 
    
    return str;
}

/// Get the version string for this firmware.  Currently we just return a 
/// string constant, but this wrapper allows for alternate implementations.
const char* GetFirmwareVersion()
{
    return FIRMWARE_VERSION;
}

/// Get the board serial number.  Currently we just return the main Sitara 
/// board's serial no., but this wrapper allows for alternate implementations.
const char* GetBoardSerialNum()
{
    static char serialNo[14] = {0};
    if(serialNo[0] == 0)
    {
        memset(serialNo, 0, 14);
        int fd = open(BOARD_SERIAL_NUM_FILE, O_RDONLY);
        if(fd < 0 || lseek(fd, 16, SEEK_SET) != 16
                  || read(fd, serialNo, 12) != 12)
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(SerialNumAccess));
        serialNo[12] = '\n';
    }
    return serialNo;
}

/// Removes all the files in specified directory
bool PurgeDirectory(std::string directoryPath)
{
    struct dirent* nextFile;
    DIR* folder;
    
    folder = opendir(directoryPath.c_str());

    // opendir returns NULL pointer if argument is not an existing directory
    if (folder == NULL) return false;
    
    while (nextFile = readdir(folder))
    {
        // skip current directory and parent directory
        if (strcmp(nextFile->d_name, ".") == 0 || strcmp(nextFile->d_name, "..") == 0)
            continue;
        std::ostringstream filePath;
        filePath << directoryPath << "/" << nextFile->d_name;
        remove(filePath.str().c_str());
    }

    closedir(folder);
    return true;
}

/// Copy a file specified by sourcePath
/// If the specified destination path is a directory, use the source filename
/// as the destination filename, otherwise use filename specified in destination path
/// This function only supports the following source/destination paths:
/// sourcePath must look like /path/to/file
/// providedDestinationPath can be /some/directory, file will be copied to /some/directory/file
/// providedDestinationPath can be /some/directory/otherFile, file will be copied to /some/directory/otherFile
/// providedDestinationPath must not have trailing slash if it is a directory
/// Anything else is not supported
bool Copy(std::string sourcePath, std::string providedDestinationPath)
{
    std::ifstream sourceFile(sourcePath.c_str(), std::ios::binary);
    std::string destinationPath;
    DIR* dir;

    if (!sourceFile.is_open())
    {
#ifdef DEBUG
        std::cerr << "could not open source file (" << sourcePath << ") for copy operation" << std::endl;
#endif
        return false;
    }
   
    // opendir returns a NULL pointer if the argument is not an existing directory
    dir = opendir(providedDestinationPath.c_str());
    
    if (dir != NULL)
    {
        // providedDestinationPath is a directory, use source filename as destination filename
        closedir(dir);
        size_t startPos = sourcePath.find_last_of("/") + 1;
        std::string fileName = sourcePath.substr(startPos, sourcePath.length() - startPos);
        destinationPath = providedDestinationPath + std::string("/") + fileName;
    }
    else
    {
        // providedDestinationPath includes destination file name
        destinationPath = providedDestinationPath;
    }

    
    std::ofstream destinationFile(destinationPath.c_str(), std::ios::binary);

    if (!destinationFile.is_open())
    {
#ifdef DEBUG
        std::cerr << "could not open destination file (" << destinationPath << ") for copy operation" << std::endl;
#endif
        return false;
    }
    
    destinationFile << sourceFile.rdbuf();
    return true;
}