//  File:   ImageProcessor.cpp
//  Implements a class for processing slice images, 
//  to correct for various issues.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
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


#include <ImageProcessor.h>
#include <Hardware.h>

using namespace Magick;

ImageProcessor::ImageProcessor() :
_patternModeImage(Geometry(PATTERN_MODE_WIDTH, PATTERN_MODE_HEIGHT), "black")
{
    // enable access to the 912x1140 image used for pattern mode
    _patternModeImage.modifyImage(); 
    _patternModeImage.type(GrayscaleType);
    _pPatternModeView = new Pixels(_patternModeImage);
    _pPatternModeCache = _pPatternModeView->get(0, 0, 
                         _patternModeImage.columns(), _patternModeImage.rows()); 
}

ImageProcessor::~ImageProcessor()
{
    delete _pPatternModeView;
}

// Scale the given image by the given scale factor.
void ImageProcessor::Scale(Image* pImage, double scale)
{
    if(scale == 1.0)
        return;
    
    int origWidth  = (int) pImage->columns();
    int origHeight = (int) pImage->rows();

    // determine size of new image (rounding to nearest pixel)
    int resizeWidth =  (int)(origWidth * scale + 0.5);
    int resizeHeight = (int)(origHeight  * scale + 0.5);

    // scale the image  
    pImage->resize(Geometry(resizeWidth, resizeHeight));  

    if (scale < 1.0)
    {
        // pad the image back to full size
        pImage->extent(Geometry(origWidth, origHeight, 
                                            (resizeWidth - origWidth) / 2, 
                                            (resizeHeight - origHeight) / 2), 
                                            "black");
    }
    else if (scale > 1.0)
    {
        // crop the image back to full size
        pImage->crop(Geometry(origWidth, origHeight, 
                                            (resizeWidth - origWidth) / 2, 
                                            (resizeHeight - origHeight) / 2));
    }
}    


// Map a central portion (rotated by 45 degrees) of the given intermediate 
// to a 912x1140 pattern mode image.
Magick::Image* ImageProcessor::MapForPatternMode(Image& imageIn)
{
    Pixels inputView(imageIn);
    const PixelPacket* inputCache = inputView.getConst(0, 0, imageIn.columns(), 
                                                             imageIn.rows());
    
    for (int y = 0; y < imageIn.rows(); y ++)
    {
        // calculate parts that only depend on y
        int widthMinusYOver2 = PATTERN_MODE_WIDTH / 2 - y / 2;
        int yPlus1Mod2 = (y + 1) % 2;
        int yMinusWidth = y - PATTERN_MODE_WIDTH;        
        const PixelPacket* input = inputCache + y * imageIn.columns();

        for(int x = 0; x < imageIn.columns(); x++)
        {
            int row = x + yMinusWidth;
            if (row < 0 || row > (PATTERN_MODE_HEIGHT - 1))
                continue;   // ignore un-mappable regions

            int column = widthMinusYOver2 + (x + yPlus1Mod2) / 2;
            if (column < 0 || column > (PATTERN_MODE_WIDTH - 1))
                continue;   // ignore un-mappable regions
                      
            // copy the pixel data from (x, y) of the input image
            // into (row, column) of the output image
            (_pPatternModeCache + row * PATTERN_MODE_WIDTH + column)->green = 
                                                             (input + x)->green; 
        }
    }
    _pPatternModeView->sync();
    return &_patternModeImage;
}