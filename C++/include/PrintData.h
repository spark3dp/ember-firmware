/* 
 * File:   PrintData.h
 * Authors: Richard Greene, Jason Lefley
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
    int GetNumLayers(const std::string directory);
    SDL_Surface* GetImageForLayer(int layer);
    bool Validate(std::string directory);
    bool Stage();
    bool MovePrintData();
    bool GetSettings(std::string& settings);
    std::string GetFileName();
    bool Clear();

private:
    static int gzOpenFrontend(char* pathname, int oflags, int mode);
    bool extractGzipTar(std::string archivePath, std::string rootPath);
    static std::string GetLayerFileName(std::string directory, int layer);

private:
    std::string _fileName;
};

#endif	/* PRINTDATA_H */

