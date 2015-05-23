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
    static int GetNumLayers(const std::string directory);
    static SDL_Surface* GetImageForLayer(int layer);
    static bool Validate(std::string directory);
    bool Stage();
    bool LoadSettings();
    bool LoadSettings(std::string filename);
    bool MovePrintData();
    std::string GetFileName();
    static bool Clear();

private:
    static int gzOpenFrontend(char* pathname, int oflags, int mode);
    bool extractGzipTar(std::string archivePath, std::string rootPath);
    static std::string GetLayerFileName(std::string directory, int layer);

private:
    std::string _fileName;
};

#endif	/* PRINTDATA_H */

