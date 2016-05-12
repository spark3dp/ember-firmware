//  File:   ImageProcessorUT.cpp
//  Tests ImageProcessor
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


#include <stdlib.h>
#include <iostream>

#include <boost/scoped_ptr.hpp>

#include <ImageProcessor.h>

int mainReturnValue = EXIT_SUCCESS;

void scalingTest() 
{
    try
    {
        ImageProcessor ip;
        // load a test image
        Magick::Image image("resources/test_image.png");
        // a scale factor of 1.0 should leave it unchanged
        ip.Scale(&image, 1.0);

        Magick::Image ref("resources/test_image.png");        

        if (!image.compare(ref))
        {
            // the image has changed 
            std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Scale of 1.0 didn't leave image unchanged" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        ip.Scale(&image, 1.1);
        
        ref.read("resources/scaled_up_image.png");
        // save the scaled image & read it back to eliminate differences 
        // introduced when writing it to a file
        image.write("/tmp/temp.png");
        image.read("/tmp/temp.png");
        if (!image.compare(ref))
        {  
            // the image has not changed as expected
            std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Unexpected output with scale of 1.1, normalized max error = " << image.normalizedMaxError() << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

        image.read("resources/test_image.png");
        ip.Scale(&image, 0.9);
        ref.read("resources/scaled_down_image.png");
        image.write("/tmp/temp.png");
        image.read("/tmp/temp.png");
        if (!image.compare(ref))
        {
            // the image has not changed as expected
            std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Unexpected output with scale of 0.9, normalized max error = " << image.normalizedMaxError() << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
        
        // test with 32bpp image
        image.read("resources/test_32bpp_image.png");
        ip.Scale(&image, 1.1);
        ref.read("resources/scaled_up_32bpp_image.png");
        image.write("/tmp/temp.png");
        image.read("/tmp/temp.png");
        if (!image.compare(ref))
        {
            // the image has not changed as expected
            std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Unexpected output with scaled up 32bpp image, normalized max error = " << image.normalizedMaxError() << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }

    }
    catch(std::exception& e)
    {
        std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Got unexpected exception: " << e.what() << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;        
    }

    bool gotExpectedException = false;
    try
    {
        ImageProcessor ip;
        Magick::Image image("resources/test_image.png");
        ip.Scale(&image, -1.0);
    }
    catch (std::exception& e)
    {
        gotExpectedException = true; 
    }
    
    if (!gotExpectedException)
    {
        std::cout << "%TEST_FAILED% time=0 testname=scalingTest (ImageProcessorUT) message=Didn't get expected exception with negative scale" << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;
    }
}

void patternModeTest()
{
    try
    {
        ImageProcessor ip;
        // load a test image
        Magick::Image input("resources/patModeInput.png");
        Magick::Image* actualOutput = ip.MapForPatternMode(input);

        Magick::Image expectedOutput("resources/patModeOutput.png");
        

        if (!actualOutput->compare(expectedOutput))
        {
            // the mapped image is not what we'd expect 
            std::cout << "%TEST_FAILED% time=0 testname=patternModeTest (ImageProcessorUT) message=Unexpected output" << std::endl;
            mainReturnValue = EXIT_FAILURE;
            return;
        }
    }
    catch(std::exception& e)
    {
        std::cout << "%TEST_FAILED% time=0 testname=patternModeTest (ImageProcessorUT) message=Got unexpected exception: " << e.what() << std::endl;
        mainReturnValue = EXIT_FAILURE;
        return;        
    }
}


int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% ImageProcessorUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% scalingTest (ImageProcessorUT)" << std::endl;
    scalingTest();
    std::cout << "%TEST_FINISHED% time=0 scalingTest (ImageProcessorUT)" << std::endl;

    std::cout << "%TEST_STARTED% patternModeTest (ImageProcessorUT)" << std::endl;
    patternModeTest();
    std::cout << "%TEST_FINISHED% time=0 patternModeTest (ImageProcessorUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}

