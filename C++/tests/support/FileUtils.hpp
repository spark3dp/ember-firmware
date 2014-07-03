/* 
 * File:   FileUtils.hpp
 * Author: jasonlefley
 *
 * Created on July 2, 2014, 12:01 PM
 */

#include <stdlib.h>
#include <limits.h>
#include <fstream>

#include <utils.h>

void Touch(std::string path)
{
    std::ofstream slice(path.c_str());
    slice.close();
}

void Copy(std::string source, std::string dest)
{
    std::ifstream src(source.c_str(), std::ios::binary);
    std::ofstream dst(dest.c_str(), std::ios::binary);
    
    dst << src.rdbuf();
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

