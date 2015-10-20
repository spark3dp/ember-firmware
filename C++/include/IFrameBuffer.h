/* 
 * File:   IFrameBuffer.h
 * Author: Jason Lefley
 *
 * Created on October 15, 2015, 3:42 PM
 */

#ifndef IFRAMEBUFFER_H
#define	IFRAMEBUFFER_H

namespace Magick
{
class Image;
}

class IFrameBuffer
{
public:
    virtual ~IFrameBuffer() { }
    virtual void Attach(Magick::Image& image) = 0;
    virtual void Draw() = 0;
    virtual int Width() = 0;
    virtual int Height() = 0;
};


#endif  // IFRAMEBUFFER_H
