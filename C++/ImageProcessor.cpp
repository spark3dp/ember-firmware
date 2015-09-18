//  File:   ImageProcessor.cpp
//  Implements a class for processing slice images, 
//  to correct for various issues
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

#include "ImageProcessor.h"

ImageProcessor::ImageProcessor() :
_callee(NULL)
{
}

ImageProcessor::ImageProcessor(const ImageProcessor& orig) 
{
}

ImageProcessor::~ImageProcessor() 
{
}

// Remove any correction steps previously included
void ImageProcessor::ClearSteps()
{
    _steps.clear();
}

// Add the given step to the ones to be performed.
void ImageProcessor::IncludeStep(CorrectionStep step)
{
    // only add if it isn't already included
    if(std::find(_steps.begin(), _steps.end(), step) == _steps.end())
        _steps.push_back(step);
}
   
void ImageProcessor::LoadImageForLayer(int layer)
{
    
}

// Start processing the current image
void ImageProcessor::Start()
{
    // make sure it's not running already
    
    // pass into thread the collection of steps to be performed
    
}
  
void ImageProcessor::Stop()
{
    
}
    
void ImageProcessor::AwaitCompletion()
{
    
}
    
// perhaps should just fire an event on completion instead?
void ImageProcessor::SetCallback(ICallback* callee)
{
    _callee = callee;
}
