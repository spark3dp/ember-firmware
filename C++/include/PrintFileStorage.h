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
    std::string GetFileName() const { return _fileName; }
    std::string GetFilePath() const { return _filePath; }
    bool HasZip() const { return _foundZip; }
    bool HasTarGz() const { return _foundTarGz; }
    bool HasOneFile() const { return _foundCount == 1; }

private:
    std::string _filePath;
    std::string _fileName;
    bool _foundTarGz;
    bool _foundZip;
    int _foundCount;
};

#endif	/* PRINTFILESTORAGE_H */

