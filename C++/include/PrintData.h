/* 
 * File:   PrintData.h
 * Authors: Richard Greene, Jason Lefly
 *
 * Handles the data for the 3D model to be printed.
 * 
 * Created on June 4, 2014, 12:45 PM
 */

#ifndef PRINTDATA_H
#define	PRINTDATA_H

#include <limits.h>

#include <SDL/SDL.h>

class PrintData {
public:
    PrintData();
    virtual ~PrintData();
    static int GetNumLayers();
    static SDL_Surface* GetImageForLayer(int layer);
    bool Validate();
    bool Stage();
    bool LoadSettings();
    bool MovePrintData();
    std::string GetFileName();
    static bool Clear();

private:
    static int gzOpenFrontend(char* pathname, int oflags, int mode);
    bool extractGzipTar(std::string archivePath, std::string rootPath);

private:
    std::string _fileName;
    
};

#endif	/* PRINTDATA_H */

