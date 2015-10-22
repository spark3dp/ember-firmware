#include "mock_hardware/ImageWritingFrameBuffer.h"

#include <Magick++.h>

ImageWritingFrameBuffer::ImageWritingFrameBuffer(const std::string& outputPath) :
_outputPath(outputPath)
{

}

ImageWritingFrameBuffer::~ImageWritingFrameBuffer()
{

}

void ImageWritingFrameBuffer::Draw(Magick::Image& image)
{
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




