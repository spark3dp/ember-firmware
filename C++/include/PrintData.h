/* 
 * File:   PrintData.h
 * Author: Jason Lefley
 * 
 * Interface to various print data implementations
 * Also holds factory functions
 *
 * Created on July 16, 2015, 3:19 PM
 */

#ifndef PRINTDATA_H
#define	PRINTDATA_H

#include <string>

struct SDL_Surface;

class PrintData
{
public:
    virtual ~PrintData() {}
    virtual bool Validate() = 0;
    virtual std::string GetFileName() = 0;
    virtual bool GetFileContents(const std::string& fileName,
        std::string& contents) = 0;
    virtual bool Remove() = 0;
    virtual bool Move(const std::string& destination) = 0;
    virtual SDL_Surface* GetImageForLayer(int layer) = 0;
    virtual int GetLayerCount() = 0;
    
    static PrintData* CreateFromNewData(const std::string& downloadDirectory,
        const std::string& dataParentDirectory);
    static PrintData* CreateFromExistingData(const std::string& fileName,
        const std::string& dataParentDirectory);
};

#endif	/* PRINTDATA_H */

