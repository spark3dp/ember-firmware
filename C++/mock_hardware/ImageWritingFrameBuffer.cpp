//  File:   ImageWritingFrameBuffer.cpp
//  Frame buffer implementation that writes the contents of the frame buffer to
//  a picture file (for testing purposes)
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
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
    image.write(0, 0, _width, _height, "G", Magick::IntegerPixel, _pixels.data());
}

// Write an image to the output path with all pixels having green value set to
// specified value.
void ImageWritingFrameBuffer::Fill(uint8_t value)
{
    std::vector<uint8_t> pixels(_width * _height, value);
    Magick::Image image(_width, _height, "I", Magick::CharPixel, pixels.data());
    image.write(_outputPath);
}

// Write in image to the output path containing pixel values from the pixel
// member vector.
void ImageWritingFrameBuffer::Swap()
{
    Magick::Image image(_width, _height, "I", Magick::IntegerPixel, _pixels.data());
    image.write(_outputPath);
}
