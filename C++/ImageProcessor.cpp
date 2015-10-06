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

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <ImageProcessor.h>
#include <Settings.h>
#include <utils.h>
#include <Shared.h>
#include <Projector.h>

using namespace Magick;

ImageProcessor::ImageProcessor() :
_processingThread(0)
{
}

ImageProcessor::ImageProcessor(const ImageProcessor& orig) 
{
}

// Destructor.
ImageProcessor::~ImageProcessor() 
{
}

// Start processing the given image.  Returns false if the processing thread is
// already running or can't be created.
bool ImageProcessor::Start(Magick::Image* pImage, Projector* pProjector)
{
    // make sure it's not running already
    if (_processingThread != 0)
        return false;
    
    _imageData.pImage = pImage;
    _imageData.pProjector = pProjector;

    return pthread_create(&_processingThread, NULL, &ProcessImage, &_imageData) == 0;  
}
  
// Stop processing the current image, and wait until the processing thread
// is no longer running.
void ImageProcessor::Stop()
{
    pthread_cancel(_processingThread);
    
    AwaitCompletion();
}
    
// Wait for processing to be done
void ImageProcessor::AwaitCompletion()
{
    StartStopwatch();
    
    if (_processingThread != 0)
    {
        void *result;
        pthread_join(_processingThread, &result);
        
        _processingThread = 0;
    }    
    
    std::cout << "    awaiting completion took " << StopStopwatch() << std::endl;
}

// Do the requested processing on the given image
void* ImageProcessor::ProcessImage(void *context)
{
    // make this thread high priority
    pid_t tid = syscall(SYS_gettid);
    setpriority(PRIO_PROCESS, tid, -10); 

    ImageData* pData = (ImageData*)context;
    
// StartStopwatch();

    // for now, just do image scaling if needed
    // in the future, there may be a series of processes to perform 
    // (e.g. uniformity and gamma correction as well as scaling)
    if(SETTINGS.GetDouble(IMAGE_SCALE_FACTOR) != 1.0)
    {
        int origWidth  = (int) pData->pImage->columns();
        int origHeight = (int) pData->pImage->rows();
        double scale = SETTINGS.GetDouble(IMAGE_SCALE_FACTOR);

        // determine size of new image (rounding to nearest pixel)
        int resizeWidth =  (int)(origWidth * scale + 0.5);
        int resizeHeight = (int)(origHeight  * scale + 0.5);

        // scale the image  
        pData->pImage->resize(Geometry(resizeWidth, resizeHeight));  

        if (scale < 1.0)
        {
            // pad the image back to full size
            pData->pImage->extent(Geometry(origWidth, origHeight, 
                                           (resizeWidth - origWidth) / 2, 
                                           (resizeHeight - origHeight) / 2), 
                                           "black");
        }
        else if (scale > 1.0)
        {
            // crop the image back to full size
            pData->pImage->crop(Geometry(origWidth, origHeight, 
                                         (resizeWidth - origWidth) / 2, 
                                         (resizeHeight - origHeight) / 2));
        }
    }
     
    // convert the image to a projectable format
    pData->pProjector->SetImage(pData->pImage);

// std::cout << "    processing took " << StopStopwatch() << std::endl;
    
    pthread_exit(NULL);
}
