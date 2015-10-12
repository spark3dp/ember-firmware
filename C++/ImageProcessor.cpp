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
#include <PrintData.h>

using namespace Magick;

Magick::Image ImageProcessor::_image;
ErrorCode ImageProcessor::_error = Success;
const char* ImageProcessor::_errorMsg = NULL;

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

// Open and start processing the image for the given layer.  If saveFile isn't 
// NULL, the processed image is saved there (for test purposes).  Returns false 
// if the processing thread is already running or can't be created.
bool ImageProcessor::Start(PrintData* pPrintData, int layer, 
                           Projector* pProjector, const char* saveFile)
{
    _error = Success;
    _errorMsg = NULL;
    
    // make sure it's not running already
    if (_processingThread != 0)
    {
        _error = IPThreadAlreadyRunning;
        return false;
    }
    
    _imageData.pPrintData = pPrintData;
    _imageData.layer = layer;
    _imageData.pProjector = pProjector;
    _imageData.imageScaleFactor = SETTINGS.GetDouble(IMAGE_SCALE_FACTOR);
    _imageData.saveFile = saveFile;

    if (pthread_create(&_processingThread, NULL, &Process, &_imageData) != 0)
    {
        _error = CantStartIPThread;
        return false;
    }
    return true;
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
//    StartStopwatch();
    
    if (_processingThread != 0)
    {
        void *result;
        pthread_join(_processingThread, &result);
        
        _processingThread = 0;
    }    
    
//    std::cout << "    awaiting completion took " << StopStopwatch() << std::endl;
}

// Do the requested processing on the given image.  Do not access Settings here,
// as they are not thread safe.
void* ImageProcessor::Process(void *context)
{
    try
    {
        // make this thread high priority
        pid_t tid = syscall(SYS_gettid);
        setpriority(PRIO_PROCESS, tid, -10); 

        ImageData* pData = (ImageData*)context;

        if (!pData->pPrintData->GetImageForLayer(pData->layer, _image))
        {
            _error = NoImageForLayer;
            pthread_exit(NULL);
        }

        // for now, just do image scaling if needed
        // in the future, there may be a series of processes to perform 
        // (e.g. uniformity and gamma correction as well as scaling)
        if (!pData->imageScaleFactor != 1.0)
        {
            int origWidth  = (int) _image.columns();
            int origHeight = (int) _image.rows();
            double scale = pData->imageScaleFactor;

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
                                            (resizeHeight - origHeight) / 2), 
                                            "black");
            }
            else if (scale > 1.0)
            {
                // crop the image back to full size
                _image.crop(Geometry(origWidth, origHeight, 
                                            (resizeWidth - origWidth) / 2, 
                                            (resizeHeight - origHeight) / 2));
            }
        }

        if (pData->saveFile != NULL)
        {
            _image.page(Geometry(0, 0));  // remove any offset from PNG file
            _image.write(pData->saveFile);
        }

        // convert the image to a projectable format
        pData->pProjector->SetImage(&_image);
    }
    catch(std::exception& e)
    {
        _error = ImageProcessing;
        _errorMsg = e.what(); 
    }
    
    pthread_exit(NULL);
}
