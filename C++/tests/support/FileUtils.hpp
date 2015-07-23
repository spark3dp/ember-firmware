/* 
 * File:   FileUtils.hpp
 * Author: jasonlefley
 *
 * Created on July 2, 2014, 12:01 PM
 */

#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>

#include <utils.h>

void Touch(std::string path)
{
    std::ofstream slice(path.c_str());
    slice.close();
}

void RemoveDir(std::string path)
{
    PurgeDirectory(path);
    remove(path.c_str());
}

std::string CreateTempDir()
{
    std::string tempDir;
    char tempDirTemplate[] = "/tmp/XXXXXX";
    tempDir = mkdtemp(tempDirTemplate);
    return tempDir;
}

/// Return the number of entries contained in the specified directory of specified type
int GetEntryCount(const std::string& directory, unsigned char type)
{
    int count = 0;
    DIR* dir = opendir(directory.c_str());
    struct dirent* entry = readdir(dir);

    while (entry != NULL)
    {
        if (entry->d_type == type && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
            count++;
        entry = readdir(dir);
    }

    closedir(dir);
    return count;
}
