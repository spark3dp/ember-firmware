/* 
 * File:   IFrameBuffer.h
 * Author: Jason Lefley
 *
 * Created on October 15, 2015, 3:42 PM
 */

#ifndef IFRAMEBUFFER_H
#define IFRAMEBUFFER_H

class IFrameBuffer
{
public:
    virtual ~IFrameBuffer() { }
    virtual void Draw(char* pixels) = 0;
    virtual int Width() = 0;
    virtual int Height() = 0;
};

#endif  // IFRAMEBUFFER_H
