/* 
 * File:   Projector.cpp
 * Author: Richard Greene
 * 
 * Encapsulates the functionality of the printer's projector.
 *
 * Created on May 30, 2014, 3:45 PM
 */

#include <iostream>

#include <SDL/SDL_image.h>

#include <Projector.h>
#include <Logger.h>
#include <PrintData.h>
#include <Filenames.h>

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
       TearDownAndExit();  // we can't  run if we can't project images
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
   
   if(_screen == NULL)
   {
       Logger::LogError(LOG_ERR, errno, SDL_SET_MODE_ERROR, SDL_GetError());
       TearDownAndExit();  // we can't  run if we can't project images       
   }
   
    // hide the cursor
    if(SDL_ShowCursor(SDL_DISABLE) != SDL_DISABLE)
    {
        // not a fatal error
        Logger::LogError(LOG_WARNING, errno, SDL_HIDE_CURSOR_ERROR, SDL_GetError());
    }
}

/// Destructor turns off projector and tears down SDL.
Projector::~Projector() 
{
    TearDown();
}

/// Load the image for the given layer.
bool Projector::LoadImageForLayer(int layer)
{
    SDL_FreeSurface(_image);
    
    _image = PrintData::GetImageForLayer(layer);
            
    // any error already logged in PrintData;
    return _image != NULL;
 
}
/// Display the previously loaded image.
bool Projector::ShowImage()
{
    if(_image == NULL)
        return false;  // no image to display
    
    if(SDL_BlitSurface(_image, NULL, _screen, NULL) != 0)
    {
        return false;
    }
    
    return SDL_Flip(_screen) == 0;    
}

/// Display an all black screen.
bool Projector::ShowBlack()
{
    if (SDL_MUSTLOCK(_screen) && SDL_LockSurface(_screen) != 0)
            return false;
    
    // fill the screen with black
    if(SDL_FillRect(_screen, NULL, 0) != 0)
        return false;
  
    if (SDL_MUSTLOCK(_screen))
        SDL_UnlockSurface (_screen) ;

    // display it
    return SDL_Flip(_screen) == 0;  
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

// Tear down projector and quit
void Projector::TearDownAndExit()
{
    TearDown();
    exit(-1);
}

/// Show a test pattern, to aid in focus and alignment.
void Projector::ShowTestPattern()
{
    SDL_FreeSurface(_image);
    
    _image = IMG_Load(TEST_PATTERN);
    if(_image == NULL)
    {
        Logger::LogError(LOG_WARNING, errno, LOAD_IMAGE_ERROR, TEST_PATTERN);
    }    
    
    ShowImage();
}