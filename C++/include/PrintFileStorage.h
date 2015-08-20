/* 
 * File:   PrintFileStorage.h
 * Author: Jason Lefley
 *
 * Provides functionality for finding a print file in a directory.
 * 
 * Created on August 19, 2015, 4:17 PM
 */

#ifndef PRINTFILESTORAGE_H
#define	PRINTFILESTORAGE_H

#include <string>

class PrintFileStorage
{
public:
    PrintFileStorage(const std::string& directory);
    ~PrintFileStorage();
    std::string GetFileName() { return _fileName; }
    std::string GetFilePath() { return _filePath; }
    bool HasZip() { return _foundZip; }
    bool HasTarGz() { return _foundTarGz; }
    bool HasFile() { return _foundTarGz || _foundZip; }

private:
    std::string _filePath;
    std::string _fileName;
    bool _foundTarGz;
    bool _foundZip;
};

#endif	/* PRINTFILESTORAGE_H */

