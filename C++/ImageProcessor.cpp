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

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <ImageProcessor.h>
#include <Settings.h>
#include <utils.h>
#include <Shared.h>

using namespace Magick;

// Gets the ImageProcessor singleton
ImageProcessor& ImageProcessor::Instance()
{
    static ImageProcessor imageProcessor;
    return imageProcessor;
}

ImageProcessor::ImageProcessor() :
_callee(NULL),
_processingThread(0),
_surface(NULL)
{
}

ImageProcessor::ImageProcessor(const ImageProcessor& orig) 
{
}

ImageProcessor::~ImageProcessor() 
{
}
 
// Load the slice image for the given layer.
// First pass here assumes the slices have 
// been extracted into separated .png files (i.e. this will not work for .zip
// print data files).  This needs instead to work with PrintData.
void ImageProcessor::LoadImage(int layer)
{
    std::cout << "loading image for layer " << layer << std::endl;
    
    // Load image directly from PNG (temporarily done here, assuming .tar.gz data)
    char path[255];
    sprintf(path, "/var/smith/print_data/%s/slice_%d.png", SETTINGS.GetString(PRINT_FILE_SETTING).c_str(), layer);
    _image.read(path);
}

// Start processing the current image.  Returns false if the procesing thread is
// already running or can't be created.
bool ImageProcessor::Start()
{
    // make sure it's not running already
    if (_processingThread != 0)
        return false;

    int retVal = pthread_create(&_processingThread, NULL, &ThreadHelper, this);  

    return 0 == retVal;
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
 //   StartStopwatch();
    
    if (_processingThread != 0)
    {
        void *result;
        pthread_join(_processingThread, &result);
        
        _processingThread = 0;
    }    
    
//    std::cout << "    awaiting completion took " << StopStopwatch() << std::endl;
}
    
// perhaps should just fire an event on completion instead?
void ImageProcessor::SetCallback(ICallback* callee)
{
    _callee = callee;
}

// Thread helper function that calls the actual processing routine
void* ImageProcessor::ThreadHelper(void *context)
{
    // make this thread high priority
    pid_t tid = syscall(SYS_gettid);
    setpriority(PRIO_PROCESS, tid, -10); 

    ImageProcessor* pip =  (ImageProcessor*)context; 
    pip->ProcessCurrentImage();
    pthread_exit(NULL);
    pip->_processingThread = 0;
}

// Do the requested processing on the current image
void ImageProcessor::ProcessCurrentImage()
{
    // for now, just do image scaling
    // in the future, there may be a series of processes to perform 
    // (e.g. uniformity and gamma correction as well as scaling)
 
 StartStopwatch();

    int origWidth  = (int) _image.columns();
    int origHeight = (int) _image.rows();
    double scale = SETTINGS.GetDouble(IMAGE_SCALE_FACTOR);
    
    // determine size of new image (rounding to nearest pixel)
    int resizeWidth =  (int)(origWidth * scale + 0.5);
    int resizeHeight = (int)(origHeight  * scale + 0.5);
    
    // scale the image  
    _image.resize(Geometry(resizeWidth, resizeHeight));  
    
    if (scale < 1.0)
    {
        // pad the image back to full size
        _image.extent(Geometry(origWidth, origHeight, 
                               (resizeWidth - origWidth) / 2, 
                               (resizeHeight - origHeight) / 2), "black");
    }
    else if (scale > 1.0)
    {
        // crop the image back to full size
        _image.crop(Geometry(origWidth, origHeight, 
                             (resizeWidth - origWidth) / 2, 
                             (resizeHeight - origHeight) / 2));
    }
     
    std::cout << "    processing took " << StopStopwatch() << std::endl;
}

