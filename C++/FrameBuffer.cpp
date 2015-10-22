#include "FrameBuffer.h"

#include <SDL/SDL.h>
#include <stdexcept>

#include "ErrorMessage.h"
#include "Logger.h"

FrameBuffer::FrameBuffer()
{
    // in case we exited abnormally before, 
    // tear down SDL before attempting to re-initialize it
    SDL_VideoQuit();
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(SdlInit, SDL_GetError()));
    }

    // use the full screen to display the images
    _videoInfo = SDL_GetVideoInfo();

    // print out video parameters
    std::cout << "screen is " << _videoInfo->current_w <<
                 " x "        << _videoInfo->current_h <<
                 " x "        << (int)_videoInfo->vfmt->BitsPerPixel << "bpp" <<
                 std::endl;
   
    _screen = SDL_SetVideoMode(_videoInfo->current_w, _videoInfo->current_h, 
                               _videoInfo->vfmt->BitsPerPixel, 
                               SDL_SWSURFACE | SDL_FULLSCREEN);

    if (!_screen)
    {
        TearDown();
        throw std::runtime_error(ErrorMessage::Format(SdlSetMode, 
                                                            SDL_GetError()));
    }
    
    // create 8 bpp surface for displaying images
    _surface = SDL_CreateRGBSurface(0, _videoInfo->current_w , 
                                       _videoInfo->current_h, 8, 0, 255, 0, 0);
    
    if (!_surface) 
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
}

FrameBuffer::~FrameBuffer()
{
    TearDown();
}

// Draw the specified pixel array to the frame buffer. Expects the pixel array
// to contain FrameBuffer::Width() * FrameBuffer::Height() 8-bit pixels (1
// element per pixel). Based on the the call to SDL_SetVideoMode() in the
// constructor, the pixel array specifies the green channel.
void FrameBuffer::Draw(char* pixels)
{
    // set the SDL surface to point to the specified pixel array
    _surface->pixels = pixels;
   
    // instruct SDL to display the surface
    if (SDL_BlitSurface(_surface, NULL, _screen, NULL) != 0)
    {
        throw std::runtime_error("unable to blit SDL surface");
    }

    if (SDL_Flip(_screen) != 0)
    {
        throw std::runtime_error("unable to flip SDL screen");
    }
   
    // clear the pixels pointer to avoid dangling pointer
    _surface->pixels = nullptr;
}

int FrameBuffer::Width()
{
    return _videoInfo->current_w;
}


int FrameBuffer::Height()
{
    return _videoInfo->current_h;
}

void FrameBuffer::TearDown()
{
    SDL_FreeSurface(_surface);
    SDL_VideoQuit();
    SDL_Quit();    
}
