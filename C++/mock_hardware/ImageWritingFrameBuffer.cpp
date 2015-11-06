#include "mock_hardware/ImageWritingFrameBuffer.h"

#include <Magick++.h>

ImageWritingFrameBuffer::ImageWritingFrameBuffer(int width, int height,
        const std::string& outputPath) :
_outputPath(outputPath),
_width(width),
_height(height),
_pixels(width * height)
{
}

ImageWritingFrameBuffer::~ImageWritingFrameBuffer()
{
}

// Copy the green channel from the specified image into the pixel member vector.
void ImageWritingFrameBuffer::Blit(Magick::Image& image)
{
    image.write(0, 0, _width, _height, "G", Magick::CharPixel, _pixels.data());
}

// Write an image to the output path with all pixels having green value set to
// specified value.
void ImageWritingFrameBuffer::Fill(char value)
{
    std::vector<char> pixels(_width * _height, value);
    Magick::Image image(_width, _height, "G", Magick::CharPixel, pixels.data());
    image.write(_outputPath);
}

// Write in image to the output path containing pixel values from the pixel
// member vector.
void ImageWritingFrameBuffer::Swap()
{
    Magick::Image image(_width, _height, "G", Magick::CharPixel, _pixels.data());
    image.write(_outputPath);
}
