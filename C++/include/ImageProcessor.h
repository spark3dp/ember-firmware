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

#define IMAGE_PROCESSOR (ImageProcessor::Instance())

#include <vector>

#include <Magick++.h>

class ImageProcessor {
public:
    static ImageProcessor& Instance();
    bool Start();
    bool LoadImage(int layer);
    void Stop();
    void AwaitCompletion();
    Magick::Image& GetImage() { return _image; }
         
private:
    pthread_t _processingThread;
    Magick::Image _image;
    
private:
    ImageProcessor();
    ImageProcessor(const ImageProcessor& orig);
    ImageProcessor& operator=(ImageProcessor const&);
    ~ImageProcessor();
    static void* ThreadHelper(void *context);
    void ProcessCurrentImage();
};

#endif	// IMAGEPROCESSOR_H 

