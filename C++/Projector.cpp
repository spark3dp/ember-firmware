//  File:   Projector.cpp
//  Encapsulates the functionality of the printer's projector
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <stdexcept>

#include <SDL/SDL_image.h>

#include <Projector.h>
#include <Logger.h>
#include <Filenames.h>
#include <MessageStrings.h>
#include <Settings.h>
#include <utils.h>

#define ON  (true)
#define OFF (false)

// Public constructor sets up SDL, base class tries to set up I2C connection 
Projector::Projector(unsigned char slaveAddress, int port) :
I2C_Device(slaveAddress, port)
{
    // see if we have an I2C connection to the projector
    _canControlViaI2C = (Read(PROJECTOR_HW_STATUS_REG) != ERROR_STATUS);
    if (!_canControlViaI2C)
        LOGGER.LogMessage(LOG_INFO, LOG_NO_PROJECTOR_I2C);
    else
    {
        // disable the projector's gamma correction, to provide linear output
        unsigned char disable = PROJECTOR_GAMMA_DISABLE;
        Write(PROJECTOR_GAMMA, &disable, 1);
    }

   // in case we exited abnormally before, 
   // tear down SDL before attempting to re-initialize it
   SDL_VideoQuit();
    
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(SdlInit, SDL_GetError()));
   }
     
   // use the full screen to display the images
   const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

    // print out video parameters
    std::cout << "screen is " << videoInfo->current_w <<
                 " x "        << videoInfo->current_h <<
                 " x "        << (int)videoInfo->vfmt->BitsPerPixel << "bpp" <<
                 std::endl; 
   
    _screen = SDL_SetVideoMode (videoInfo->current_w, videoInfo->current_h, 
                                videoInfo->vfmt->BitsPerPixel, 
                                SDL_SWSURFACE | SDL_FULLSCREEN) ;   
   
    if (_screen == NULL)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(SdlSetMode, 
                                                            SDL_GetError()));
    }
    
    // create 8 bpp surface for displaying images
    _surface = SDL_CreateRGBSurface(0, videoInfo->current_w , 
                                       videoInfo->current_h, 8, 0, 255, 0, 0);
    
    if (_surface == NULL) 
    {   
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(SDLCreateSurface, 
                                                            SDL_GetError()));
    }
   
    // hide the cursor
    SDL_ShowCursor(SDL_DISABLE);
    if (SDL_ShowCursor(SDL_QUERY) != SDL_DISABLE)
    {
        // not a fatal error
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(SdlHideCursor), 
                                                            SDL_GetError());
    }
            
    ShowBlack();
     
    Magick::InitializeMagick("");
}

// Destructor turns off projector and tears down SDL.
Projector::~Projector() 
{
    TearDown();
}

// Convert the given image to a displayable surface.
void Projector::SetImage(Magick::Image* pImage)
{
    pImage->write(0, 0, pImage->columns(), pImage->rows(), "G", 
                  Magick::CharPixel, _surface->pixels);
}

// Display the given image (or _image if given image is NULL).
bool Projector::ShowImage(SDL_Surface* surface)
{
    if(surface == NULL)
        surface = _surface;
    
    if (surface == NULL || SDL_BlitSurface(surface, NULL, _screen, NULL) != 0)
        return false;     
    
    TurnLED(ON);
    
    return SDL_Flip(_screen) == 0;    
}

// Display an all black screen.
bool Projector::ShowBlack()
{
    TurnLED(OFF);

    if (SDL_MUSTLOCK(_screen) && SDL_LockSurface(_screen) != 0)
            return false;
    
    // fill the screen with black
    if (SDL_FillRect(_screen, NULL, 0) != 0)
        return false;
  
    if (SDL_MUSTLOCK(_screen))
        SDL_UnlockSurface(_screen) ;

    // display it
    return SDL_Flip(_screen) == 0;  
}

// Display an all white screen.
bool Projector::ShowWhite()
{
    TurnLED(ON);

    if (SDL_MUSTLOCK(_screen) && SDL_LockSurface(_screen) != 0)
            return false;
    
    // fill the screen with white
    if (SDL_FillRect(_screen, NULL, 0xFFFFFFFF) != 0)
        return false;
  
    if (SDL_MUSTLOCK(_screen))
        SDL_UnlockSurface (_screen) ;

    // display it
    return SDL_Flip(_screen) == 0;  
}

// Turn off projector and tear down SDL
void Projector::TearDown()
{
    ShowBlack();
    SDL_FreeSurface(_surface);
    SDL_VideoQuit();
    SDL_Quit();    
}

// Show a test pattern, to aid in focus, alignment, and/or calibration.
void Projector::ShowTestPattern(const char* path)
{    
    SDL_Surface* image = IMG_Load(path);
    if (image == NULL)
    {
        LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(LoadImageError), path);
    }    
    
    ShowImage(image);
    SDL_FreeSurface(image);
}

// Turn the projector's LED(s) on or off.  Set the current to the desired value 
// first when turning them on.
void Projector::TurnLED(bool on)
{   
    if (!_canControlViaI2C)
        return;
    
    if (on)
    {
        // set the LED current, if we have a valid setting value for it
        int current = SETTINGS.GetInt(PROJECTOR_LED_CURRENT);
        if (current > 0)
        {
            // Set the PWM polarity.
            // Though the PRO DLPC350 Programmer’s Guide says to set this after 
            // setting the LED currents, it appears to need to be set first.
            // Also, the Programmer’s Guide seems to have the 
            // polarity backwards.
            unsigned char polarity = PROJECTOR_PWM_POLARITY_NORMAL;
            Write(PROJECTOR_LED_PWM_POLARITY_REG, &polarity, 1);
            
            unsigned char c = (unsigned char) current;
            // use the same value for all three LEDs
            unsigned char buf[3] = {c, c, c};

            Write(PROJECTOR_LED_CURRENT_REG, buf, 3);
        }
    }
    
    Write(PROJECTOR_LED_ENABLE_REG, on ? PROJECTOR_ENABLE_LEDS : 
                                         PROJECTOR_DISABLE_LEDS);
}
