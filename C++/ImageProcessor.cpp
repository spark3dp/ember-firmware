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
