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
#include <MessageStrings.h>

#define ON  (true)
#define OFF (false)

/// Public constructor sets up SDL, base class tries to set up I2C connection 
Projector::Projector(unsigned char slaveAddress, int port) :
I2C_Device(slaveAddress, port),
_image(NULL)
{
    // see if we have an I2C connection to the projector
    _canControlViaI2C = (Read(PROJECTOR_HW_STATUS_REG) != (unsigned char)-1);
    if(!_canControlViaI2C)
        LOGGER.LogMessage(LOG_INFO, LOG_NO_PROJECTOR_I2C);

   // in case we exited abnormally before, 
   // tear down SDL before attempting to re-initialize it
   SDL_VideoQuit();
    
   if(SDL_Init(SDL_INIT_VIDEO) < 0)
   {
       LOGGER.LogError(LOG_ERR, errno, ERR_MSG(SdlInit), SDL_GetError());
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
       LOGGER.LogError(LOG_ERR, errno, ERR_MSG(SdlSetMode), SDL_GetError());
       TearDownAndExit();  // we can't  run if we can't project images       
   }
   
    // hide the cursor
    SDL_ShowCursor(SDL_DISABLE);
    if(SDL_ShowCursor(SDL_QUERY) != SDL_DISABLE)
    {
        // not a fatal error
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(SdlHideCursor), SDL_GetError());
    }
            
    ShowBlack();
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
    
    TurnLED(ON);
    
    return SDL_Flip(_screen) == 0;    
}

/// Display an all black screen.
bool Projector::ShowBlack()
{
    TurnLED(OFF);

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

/// Turn off projector and tear down SDL
void Projector::TearDown()
{
    ShowBlack();
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
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(LoadImageError), TEST_PATTERN);
    }    
    
    ShowImage();
}

/// Show a projector calibration image, to aid in alignment.
void Projector::ShowCalibrationPattern()
{
    SDL_FreeSurface(_image);
    
    _image = IMG_Load(CAL_IMAGE);
    if(_image == NULL)
    {
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(LoadImageError), CAL_IMAGE);
    }    
    
    ShowImage();
}

/// Turn the projector's LED(s) on or off.
void Projector::TurnLED(bool on)
{   
    if(!_canControlViaI2C)
        return;
    
    Write(PROJECTOR_LED_ENABLE_REG, on ? PROJECTOR_ENABLE_LEDS : 
                                         PROJECTOR_DISABLE_LEDS);
}
