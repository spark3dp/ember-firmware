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

using namespace Magick;

ImageProcessor::ImageProcessor() :
_patternModeImage("912x1140", "black")
{
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
    
    // create final 912x1140 image 
    _patternModeImage.erase();
    _patternModeImage.modifyImage();       
    _patternModeImage.type(imageIn.type());
    Pixels viewOut(_patternModeImage);
    PixelPacket* outCache = viewOut.get(0, 0, 912, 1140); 
        
    for (int y = 0; y < imageIn.rows(); y ++)
    {
        int widthMinusYOver2 = 912 / 2 - y / 2;
        int yPlus1Mod2 = (y + 1) % 2;
        int yMinusWidth = y - 912;
        
        for(int x = 0; x < imageIn.columns(); x++)
        {
            int row = x + yMinusWidth;
            if (row < 0 || row > 1139)
                continue;   // ignore un-mappable regions

            int col = widthMinusYOver2 + (x + yPlus1Mod2) / 2;
            if (col < 0 || col > 911)
                continue;   // ignore un-mappable regions
                      
            // copy the pixel data from (x,y) of the input image
            // into (row,col) of the output image
            *(outCache + row * 912 + col) = imageIn.pixelColor(x, y);
        }
    }
    viewOut.sync();
    
    return &_patternModeImage;
}