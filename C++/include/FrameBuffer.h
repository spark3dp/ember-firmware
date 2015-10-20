/* 
 * File:   FrameBuffer.h
 * Author: Jason Lefley
 *
 * Created on October 15, 2015, 4:07 PM
 */

#ifndef FRAMEBUFFER_H
#define	FRAMEBUFFER_H

#include "IFrameBuffer.h"

struct SDL_Surface;
struct SDL_VideoInfo;

class FrameBuffer : public IFrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();
    void Attach(Magick::Image& image);
    void Draw();
    int Width();
    int Height();

private:
    void TearDown();

    SDL_Surface* _screen;
    SDL_Surface* _surface;
    const SDL_VideoInfo* _videoInfo;
};


#endif  // FRAMEBUFFER_H

