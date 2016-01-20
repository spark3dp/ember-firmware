//  File:   FrameBuffer.h
//  Encapsulates access to frame buffer through SDL, providing interface to
//  video hardware
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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
    void Blit(Magick::Image& image);
    void Fill(unsigned int value);
    void Swap();

private:
    void TearDown();

    SDL_Surface* _screen;
    SDL_Surface* _surface;
    const SDL_VideoInfo* _videoInfo;
};


#endif  // FRAMEBUFFER_H

