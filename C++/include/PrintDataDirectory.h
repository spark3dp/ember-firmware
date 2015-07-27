/* 
 * File:   PrintDataDirectory.h
 * Authors: Richard Greene, Jason Lefley
 *
 * Handles data stored in a directory for the 3D model to be printed.
 * 
 * Created on June 4, 2014, 12:45 PM
 */

#ifndef PRINTDATADIRECTORY_H
#define	PRINTDATADIRECTORY_H

#include <PrintData.h>

class PrintDataDirectory : public PrintData
{
public:
    PrintDataDirectory(const std::string& fileName, const std::string& dataDirectory);
    virtual ~PrintDataDirectory();
    bool Validate();
    std::string GetFileName();
    bool GetFileContents(const std::string& fileName, std::string& contents);
    bool Remove();
    bool Move(const std::string& destination);
    SDL_Surface* GetImageForLayer(int layer);
    int GetLayerCount();

private:
    std::string GetLayerFileName(int layer);

private:
    std::string _fileName;      // The name of the file originally containing the print data
    std::string _dataDirectory; // The directory containing the print data
};

#endif	/* PRINTDATADIRECTORY_H */

