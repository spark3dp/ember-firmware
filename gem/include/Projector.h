/* 
 * File:   Projector.h
 * Author: Richard Greene
 * 
 * Encapsulates the functionality of the printer's projector.
 *
 * Created on May 30, 2014, 3:45 PM
 */

#ifndef PROJECTOR_H
#define	PROJECTOR_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

class Projector {
public:
    Projector();
    virtual ~Projector();
    bool LoadImage(char* filename);
    void ShowImage();
    void ShowBlack();
    void SetPowered(bool on);
    
private:
    SDL_Surface* _screen;
    SDL_Surface* _image ;
};

#endif	/* PROJECTOR_H */

