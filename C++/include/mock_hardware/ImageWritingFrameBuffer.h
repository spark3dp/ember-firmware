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
#include <vector>

class ImageWritingFrameBuffer : public IFrameBuffer
{
public:
    ImageWritingFrameBuffer(int width, int height, const std::string& outputPath);
    ~ImageWritingFrameBuffer();
    void Blit(Magick::Image& image);
    void Fill(char value);
    void Swap();
    
private:
    const std::string _outputPath;
    int _width;
    int _height;
    std::vector<char> _pixels;
};

#endif  // MOCKHARDWARE_IMAGEWRITINGFRAMEBUFFER_H

