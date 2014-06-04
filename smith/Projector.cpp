/* 
 * File:   Projector.cpp
 * Author: Richard Greene
 * 
 * Encapsulates the functionality of the printer's projector.
 *
 * Created on May 30, 2014, 3:45 PM
 */

#include <iostream>

#include <Projector.h>
#include <Logger.h>

/// Constructor sets up SDL.
Projector::Projector() :
_image(NULL)
{
   // in case we exited abnormally before, 
   // tear down SDL before attempting to re-initialize it
   SDL_VideoQuit();
    
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
       Logger::LogError(LOG_ERR, errno, SDL_INIT_ERROR, SDL_GetError());
       exit(-1);  // we can't  run if we can't project images
   }
     
   // use the full screen to display the images
   const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
#ifdef DEBUG
    // print out video parameters
    std::cout << "screen is " << videoInfo->current_w <<
                 " x "        << videoInfo->current_h <<
                 " x "        << (int)videoInfo->vfmt->BitsPerPixel << "bpp" <<
                 std::endl; 
#endif
   
   _screen = SDL_SetVideoMode (videoInfo->current_w, videoInfo->current_h, 
                               videoInfo->vfmt->BitsPerPixel, 
                               SDL_SWSURFACE | SDL_FULLSCREEN) ;   
   
   // TODO: handle error if _screen is NULL
   
    // hide the cursor
    SDL_ShowCursor(SDL_DISABLE);
}

/// Destructor turns off projector and tears down SDL.
Projector::~Projector() 
{
    TearDown();
}

/// Open an image from a PNG file.
bool Projector::LoadImage(char* filename)
{
    SDL_FreeSurface(_image);
    
    _image = IMG_Load(filename);
    if(_image == NULL)
    {
        Logger::LogError(LOG_ERR, errno, LOAD_IMAGE_ERROR, filename);
        return false;
    }
    return true;
}

/// Display the previously loaded image.
void Projector::ShowImage()
{
    if(_image == NULL)
        return;  // no image to display
    
    SDL_BlitSurface(_image, NULL, _screen, NULL);
    SDL_Flip(_screen);    
}

/// Display an all black screen.
void Projector::ShowBlack()
{
    if (SDL_MUSTLOCK(_screen)) 
        SDL_LockSurface(_screen);
    
    // fill the screen with black
    SDL_FillRect(_screen, NULL, 0);
  
    if (SDL_MUSTLOCK(_screen))
        SDL_UnlockSurface (_screen) ;

    // display it
    SDL_Flip(_screen);
}

/// Turn the projector on or off.
void Projector::SetPowered(bool on)
{
  // TODO: control the projector over I2C  
}

/// Turn off projector and tear down SDL
void Projector::TearDown()
{
    ShowBlack();
    SetPowered(false);
    SDL_FreeSurface(_image);
    SDL_VideoQuit();
    SDL_Quit();    
}