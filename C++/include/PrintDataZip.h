/* 
 * File:   PrintDataZip.h
 * Author: Jason Lefley
 *
 * Handles data stored in a zip file for the 3D model to be printed.
 *
 * Created on July 16, 2015, 5:49 PM
 */

#ifndef PRINTDATAZIP_H
#define	PRINTDATAZIP_H

#include <zpp.h>

#include <PrintData.h>

class PrintDataZip : public PrintData
{
public:
    PrintDataZip(const std::string& fileName, const std::string& filePath);
    virtual ~PrintDataZip();
    bool Validate();
    std::string GetFileName();
    bool GetFileContents(const std::string& fileName, std::string& settings);
    bool Remove();
    bool Move(const std::string& destination);
    SDL_Surface* GetImageForLayer(int layer);
    int GetLayerCount();

    static void Initialize();

private:
    std::string GetLayerFileName(int layer);

private:
    std::string _fileName;     // The name of the file originally containing the print data
    std::string _filePath;     // The path to the zip file backing this instance
    zppZipArchive _zipArchive; // zpp zip archive wrapper
};

#endif	/* PRINTDATAZIP_H */

