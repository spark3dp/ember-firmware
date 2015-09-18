//  File:   ImageProcessor.h
//  Defines a class for processing slice images, to correct for various issues
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

#ifndef IMAGEPROCESSOR_H
#define	IMAGEPROCESSOR_H

#include <vector>

#include "ICallback.h"

enum CorrectionStep
{
    // scale an image, e.g. to correct for misalignment of the projector
    Scale,
    
    // correct for spatial inhomogeneity of the projected light
    Uniformity,
    
    // correct for non-linearities in the dose/response curve, e.g. for
    // a particular resin
    Gamma,  
};

class ImageProcessor {
public:
    ImageProcessor();
    ImageProcessor(const ImageProcessor& orig);
    virtual ~ImageProcessor();
    void ClearSteps();
    void IncludeStep(CorrectionStep step);
    void Start();
    void LoadImageForLayer(int layer);
    void Stop();
    void AwaitCompletion();
    void SetCallback(ICallback* callee);  
    
private:
    ICallback* _callee;
    std::vector<CorrectionStep> _steps;
};

#endif	// IMAGEPROCESSOR_H 

