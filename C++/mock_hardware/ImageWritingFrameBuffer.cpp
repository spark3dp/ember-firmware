#include "mock_hardware/ImageWritingFrameBuffer.h"

#include <Magick++.h>

ImageWritingFrameBuffer::ImageWritingFrameBuffer(const std::string& outputPath) :
_outputPath(outputPath)
{

}

ImageWritingFrameBuffer::~ImageWritingFrameBuffer()
{

}

// Write the specified pixel array to an image file.
void ImageWritingFrameBuffer::Draw(char* pixels)
{
    Magick::Image image(Width(), Height(), "G", Magick::CharPixel, pixels);
    image.write(_outputPath);
}

int ImageWritingFrameBuffer::Width()
{
    return 1280;
}

int ImageWritingFrameBuffer::Height()
{
    return 800;
}




