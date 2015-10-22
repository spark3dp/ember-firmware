/* 
 * File:   ImageWritingFrameBuffer.h
 * Author: Jason Lefley
 *
 * Created on October 21, 2015, 4:52 PM
 */

#ifndef MOCKHARDWARE_IMAGEWRITINGFRAMEBUFFER_H
#define MOCKHARDWARE_IMAGEWRITINGFRAMEBUFFER_H

#include "IFrameBuffer.h"

#include <string>

class ImageWritingFrameBuffer : public IFrameBuffer
{
public:
    ImageWritingFrameBuffer(const std::string& outputPath);
    ~ImageWritingFrameBuffer();
    void Draw(Magick::Image& image);
    int Width();
    int Height();
    
private:
    const std::string _outputPath;
};

#endif  // MOCKHARDWARE_IMAGEWRITINGFRAMEBUFFER_H

