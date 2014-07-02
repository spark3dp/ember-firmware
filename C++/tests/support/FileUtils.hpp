/* 
 * File:   FileUtils.hpp
 * Author: jasonlefley
 *
 * Created on July 2, 2014, 12:01 PM
 */

#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <fstream>

void Touch(std::string parent, std::string name)
{
    // Create an empty file with specified name in specified parent directory
    std::string fullPath = parent + "/" + name;
    std::ofstream slice(fullPath.c_str());
    slice.close();
}

void Purge(std::string directory)
{
    struct dirent* nextFile;
    DIR* folder;
    char filePath[PATH_MAX];
    
    folder = opendir(directory.c_str());
    
    while (nextFile = readdir(folder))
    {
        sprintf(filePath, "%s/%s", directory.c_str(), nextFile->d_name);
        remove(filePath);
    }
}

void Copy(std::string source, std::string dest)
{
    std::ifstream src(source.c_str(), std::ios::binary);
    std::ofstream dst(dest.c_str(), std::ios::binary);
    
    dst << src.rdbuf();
}

void RemoveDir(std::string path)
{
    Purge(path);
    remove(path.c_str());
}

std::string CreateTempDir()
{
    std::string tempDir;
    char tempDirTemplate[] = "/tmp/XXXXXX";
    tempDir = mkdtemp(tempDirTemplate);
    return tempDir;
}

