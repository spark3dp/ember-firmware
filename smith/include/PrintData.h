/* 
 * File:   PrintData.h
 * Author: Richard Greene
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
    
private:

};

#endif	/* PRINTDATA_H */

