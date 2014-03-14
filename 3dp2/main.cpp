/* 
 * File:   main.cpp
 * Author: Richard Greene
 * Port of original 3d.c for Raspberry Pi, with minimum needed changes 
 * Created on March 6, 2014, 4:54 PM
 */


#ifndef	TRUE
#  define	TRUE  (1==1)
#  define	FALSE (!(TRUE))
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <cv.h>
#include <ml.h>
#include <cxcore.h>
#include <highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
using namespace std;
#include <cstdlib>

using namespace cv;

// Main program globals

// Exposure time for normal layers
int exposureTime = 1000 ;	// 1000 milliseconds

//Exposure time for first layer
int firstExposureTime = 5000 ; //5,000 milliseconds

//Exposure time for calibration layers
int calibrationExposureTime = 3000 ; //3000 milliseconds

//Exposure time for support layers
int supportExposureTime = 250 ; //250 milliseconds

//Exposure time for  perimieter of model layers
int perimeterTime = 1000; //1000 milliseconds

//Number of calibration layers
int numCalLayers = 4 ;

//Number of support layers
int numSupLayers = 6 ;

//Layer Thickness

unsigned int sliceThickness;

//Calibration layer thickness
//int calThickness = 25; //25 microns

int useArduino   = TRUE ;
int serialFd ;

// RG additions
IplImage* blackScreen = NULL;
IplImage* image[2] = {NULL, NULL};
const int screenWidth = 1280 ;
const int screenHeight = 800;
const char* windowName = "3dp2";
int nImage = 0;

/*
 * failUsage:
 *	Program usage information
 *********************************************************************************
 */

const char *usage = "Usage: %s\n"
        "   [-h]            # This help information\n"
        "   [-x]            # No Arduino\n"
        "   [-f time]       # First layer xposure time in mS\n"
        "   [-c time]       # Calibration layer exposure time in mS\n"
        "   [-s time]       # Support layer exposure time in mS\n"
        "   [-t time]       # Exposure time in mS\n"
        "   [-C layers]     # Number of calibration layers\n"
        "   [-S layers]     # Number of support layers\n"
        "   [-d device]     # Serial Device of Arduino (/dev/ttyACM default for Uno)\n"
        "   [-m microsteps]     # Microstepping factor: 1..6\n"
        "   [-l sliceThickness] # Thickness of each slice in 1/1000 mm\n"
        "   fileNameTemplate    # Template - must have 0000 in it\n";

static void failUsage (char *progName)
{
  fprintf (stderr, usage, progName) ;
  exit (EXIT_FAILURE) ;
}


//stub methods for serial port, to test imaging functions first
void serialPrintf (int, const char*, int) {}
void serialPutchar (int, char) {}
char serialGetchar (int){}

// Display an image and pause some milliseconds for its display
void showImage(IplImage* image, int pause_msec = 10)
{
    cvShowImage(windowName, image);
    cvWaitKey(pause_msec);
}

/*
 * screenClear:
 *	Clear to black (with initialization of window on first call)
 *********************************************************************************
 */
void screenClear (void)
{
    if(!blackScreen)
    {
        // initialize the all black image
        CvSize imageSize;
        imageSize.width = screenWidth;
        imageSize.height = screenHeight;
        blackScreen = cvCreateImage(imageSize, IPL_DEPTH_8U, 1);
        cvZero(blackScreen);
        cvNamedWindow(windowName, CV_WINDOW_AUTOSIZE);
        showImage(blackScreen, 500); // need long pause first time
    }
    else
        showImage(blackScreen);
}



// load an image from a file, and make its alpha layer visible
IplImage* loadImage(const char* fileName)
{
  IplImage* img  = cvLoadImage(fileName, -1); // need -1 to preserve alpha channel!
  
  if (img == NULL)
  {
    printf ("\nMissing image: %s\n", fileName) ;
    return NULL;
  }
  
  // convert the alpha channel to gray values in the RGB channels
  // assume it's a monochrome image to start with
  // with pure white and black pixels needing no further work
  uint8_t* pixelPtr = (uint8_t*)img->imageData;
  int cn = img->nChannels;
  for(int i = 0; i < img->height; i++)
  {
    int yOffset = i * img->width * cn;
    for(int j = 0; j < img->width; j++)
    {
       int xOffset = j * cn;
       unsigned char alpha = pixelPtr[yOffset + xOffset + 3];
       if(alpha != 0 && alpha != 255)
       {
          pixelPtr[yOffset + xOffset] = alpha; 
          pixelPtr[yOffset + xOffset + 1] = alpha;
          pixelPtr[yOffset + xOffset + 2] = alpha;
       }
    }
  }
  return img;
}
/*
 * videoTest:
 *	Display simple test pattern
 *********************************************************************************
 */

void videoTest (void)
{
    screenClear();
    
    // get the path to this exe
    char buff[1024];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) 
    {
      buff[len] = '\0';
    } else 
    {
        printf("could not get path in which this exe resides\n");
    }

    // remove the exe's name & replace with the test pattern file name
    std:string path = buff;
    int posn = path.find_last_of('/');
    path.resize(posn);
    path.append("/TestPattern.png");
    IplImage* testPat = loadImage(path.c_str());
    if(!testPat)
        return;
    
    showImage(testPat);
    
    // wait for key press
    while(cvWaitKey(100) < 0)
        ;
    
    cvReleaseImage(&testPat); 
}


/*
 * checkTemplate:
 *	Check the supplied filename template and make sure the first file
 *	actually exists!
 *********************************************************************************
 */
void checkTemplate (char *progName, char *filenameTemplate)
{
  int  fd, i ;
  int  found = FALSE ;
  char *p, fileName [1024] ;

  if (strlen (filenameTemplate) < 8)	// 0000.png = 8 chars, so minimum filename
  {
    fprintf (stderr, "%s: Filename template \"%s\" is too short\n", progName, filenameTemplate) ;
    exit (EXIT_FAILURE) ;
  }

  for (p = filenameTemplate, i = 0 ; i < strlen (filenameTemplate) - 4 ; ++i)
    if (strncmp (p, "0000", 4) == 0)
    {
      found = TRUE ;
      break ;
    }
    else
      ++p ;

  if (!found)
  {
    fprintf (stderr, "%s: No 0000 found in filename template \"%s\"\n", progName, filenameTemplate) ;
    exit (EXIT_FAILURE) ;
  }

  *p++ = '%' ; *p++ = '0' ; *p++ = '4' ; *p++ = 'd' ;

  sprintf (fileName, filenameTemplate, 1) ;
  if ((fd = open (fileName, O_RDONLY)) < 0)
  {
    fprintf (stderr, "%s: Unable to open first file: \"%s\": %s\n", progName, fileName, strerror (errno)) ;
    exit (EXIT_FAILURE) ;
  }
  close (fd) ;
}

long getMillis(){
    struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
    // printf("time = %d sec + %ld nsec\n", now.tv_sec, now.tv_nsec);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/*
 * processImages:
 *	Do all the work to process and print our images
 *********************************************************************************
 */
void processImages (char *progName, char *filenameTemplate)
{
  char fileName [1024] ;
  int   i ;
  unsigned long start = 0L, timeUp, time, cycleTime = 0L;
  int   j = 2 + numCalLayers;
  int   k = j + numSupLayers;
  std:string frameType;

// Pre-load first image

  sprintf (fileName, filenameTemplate, 1) ;
  image[nImage] = loadImage(fileName);
  if(!image)
      return;
  
//Calibration sequence
// Send command to Arduino to calibrate the mechanicals

   if (useArduino)
      serialPutchar (serialFd, 'c') ;
      //serialGetchar (serialFd) ;
      //printf ("serialFd: %d", serialFd);

  usleep (45000);

// Get serial signal from the arduino to signal that it's stopped moving

    if (useArduino)
      serialGetchar (serialFd) ;

//Cycle through images

  for (i = 2 ; i < 9999 ; ++i)
  {

// If statments to determine correct exposure time for each layer
// First layer

    if (i == 2)
    {
      time = firstExposureTime ;
      frameType = "First Layer" ;
    }

//Calibration layers

    else if (i > 2 && i <= j)
    {
      time = calibrationExposureTime ;
      frameType = "Calibration Layer" ;
    }

//Support layers

    else if (i > j && i <= k)
    {
      time = supportExposureTime ;
      frameType = "Support Layer" ;
    }

//Model layers

    else
    {
      if (k%2 ==0)
      {
	  if (i%2 != 0)
          {
            time = exposureTime ;
            frameType = "Model Layer" ;
          }

	  else
	  {
	    time = perimeterTime;
	    frameType = "Perimeter Model Layer";
          }
       }

      if (k%2 !=0)
      {
          if (i%2 == 0)
          {
            time = exposureTime ;
            frameType = "Model Layer" ;
          }

          else
          {
            time = perimeterTime;
            frameType = "Perimeter Model Layer";
          }
       }
     }
// Put the image in-hand on the display

    if(start != 0L)
        cycleTime = getMillis() - start;
    start  = getMillis() ;
    timeUp = start + time ;
    showImage(image[nImage]);
    printf ("\nDisplaying frame: %04d %s thickness:%4d  Cycle time: %4d", i - 1, frameType.c_str(), sliceThickness, cycleTime ) ; fflush (stdout) ;

// Load the next image
    if(++nImage > 1)
        nImage = 0;
    sprintf (fileName, filenameTemplate, i) ;
    cvReleaseImage(&image[nImage]);
    image[nImage] = loadImage(fileName) ;
    
// Wait for the exposure time to be up

    while (getMillis() < timeUp)
      usleep (1) ;

// Blank the display
    screenClear () ;

// Send command to Arduino to move the mechanicals

  if (i == 2)
  {
    if (useArduino)
        //Print cycle with rotation and overlift
        serialPutchar (serialFd, 'T') ;
  }

  else if (i <= k )
   {
     if (useArduino)
        //Print cycle with rotation and overlift
        serialPutchar (serialFd, 'T') ;
   }

   else
   {
     if (k%2 == 0)
     {
          if (i%2 == 0)
          {
            if (useArduino)
            //Print cycle with rotation and overlift
            serialPutchar (serialFd, 'P') ;
          }
     }

     if (k%2 !=0)
     {
          if (i%2 != 0)
          {
            if (useArduino)
            //Print cycle with rotation and overlift
            serialPutchar (serialFd, 'P') ;
          }
     }
   }


// No more images?

    if (image[nImage] == NULL)
    {
     if (useArduino)
        //Rotate Clockwise 90 degrees
        serialPutchar (serialFd, 'r') ;

     if (useArduino)
        //Home Z Axis
        serialPutchar (serialFd, 'h') ;
      printf ("\n\n%s: Out of images at: %d\n", progName, i -1) ;
      break ;
    }

// Blit the next image to the screen
    showImage(image[nImage]);
    
// Wait for the arduino to signal that it's stopped moving
   if (i == 2)
   {
      usleep (10000);
      if (useArduino)
         serialGetchar (serialFd) ;
   }


   else if (i <= k )
   {
      if (useArduino)
        serialGetchar (serialFd) ;
   }

   else
   {
     if (k%2 == 0)
     {
          if (i%2 == 0)
          {
            if (useArduino)
            serialGetchar (serialFd) ;
          }
     }

     if (k%2 !=0)
     {
          if (i%2 !=0)
          {
            if (useArduino)
            serialGetchar (serialFd) ;
          }
     }
    }

// Quick abort, on any key press
    int key = cvWaitKey(100);
    if (key > 0)
    {
        printf("key pressed: %d\n", key);
        break ;
    }

  }
  
  cvReleaseImage(&blackScreen); 
  cvReleaseImage(&image[0]);
  cvReleaseImage(&image[1]);
  cvDestroyWindow(windowName);

  printf ("\n\nRun ended after %d frames\n\n", i -1) ;

}


/*
 *********************************************************************************
 * main:
 *********************************************************************************
 */

int main (int argc, char *argv [])
{
  int  uSteps  = 1 ;
  int  opt ;
  char* filenameTemplate ;

  char* device;
 // int sliceThickness = 25 ;
  unsigned int then ;
  int ch ;


  while ((opt = getopt (argc, argv, "hxvd:m:t:f:c:s:l:C:S:p:")) != -1)
  {
    switch (opt)
    {
      case 'h':		// Heyalp
	failUsage (argv [0]) ;
	break ;

      case 'x':		// Ignore arduino for testing
	useArduino = FALSE ;
	break ;

      case 'v':		// Video Test
	videoTest () ;
	return 0 ;

      case 't':		// Exposure time normal layer
	exposureTime = atoi (optarg) ;
	if ((exposureTime < 0) || (exposureTime > 1800000))
	{
	  fprintf (stderr, "%s: Normal Layer exposure time out of range (100-1800000mS)\n", argv [0]) ;
	  exit (EXIT_FAILURE) ;
	}
	break ;

      case 'p':         // Exposure time perimeter layer
        perimeterTime = atoi (optarg) ;
        if ((perimeterTime < 0) || (perimeterTime > 1800000))
        {
          fprintf (stderr, "%s: Perimeter layer exposure time out of range (100-1800000mS)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 'f':         // Exposure time first layer
        firstExposureTime = atoi (optarg) ;
        if ((firstExposureTime < 0) || (firstExposureTime > 1800000))
        {
          fprintf (stderr, "%s: First layer exposure time out of range (100-1800000mS)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 'c':         // Exposure time calibration layer
        calibrationExposureTime = atoi (optarg) ;
        if ((calibrationExposureTime < 0) || (calibrationExposureTime > 1800000))
        {
          fprintf (stderr, "%s: Calibration layer exposure time out of range (100-1800000mS)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 's':         // Exposure time support layer
        supportExposureTime = atoi (optarg) ;
        if ((supportExposureTime < 0) || (supportExposureTime > 1800000))
        {
          fprintf (stderr, "%s: Support layer exposure time out of range (100-1800000mS)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 'C':         // Number of calibration layers
        numCalLayers = atoi (optarg) ;
        if ((numCalLayers < 0) || (numCalLayers > 20))
        {
          fprintf (stderr, "%s: Number of calibration layers  out of range (1-20)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 'S':         // Number of support layers
        numSupLayers = atoi (optarg) ;
        if ((numSupLayers < 0) || (numSupLayers > 400))
        {
          fprintf (stderr, "%s: Number of support layers  out of range (1-400)\n", argv [0]) ;
          exit (EXIT_FAILURE) ;
        }
        break ;

      case 'd':
	if (*optarg != '/')
	  failUsage (argv [0]) ;

	device = (char*)malloc (strlen (optarg) + 1) ;
	strcpy (device, optarg) ;
	break ;

      case 'm':
	uSteps = atoi (optarg) ;
	if ((uSteps < 1) || (uSteps > 6))
	  failUsage (argv [0]) ;
	break ;

      case 'l':			// Slice thickness
	sliceThickness = atoi (optarg) ;
	if ((sliceThickness > 1000) || (sliceThickness < 25))
	  failUsage (argv [0]) ;
	break ;
    }
  }

  if (optind >= argc)
    failUsage (argv [0]) ;

  filenameTemplate = (char*) malloc (strlen (argv [optind]) + 1) ;
  strcpy (filenameTemplate, argv [optind]) ;

  checkTemplate (argv [0], filenameTemplate) ;
  
  if (useArduino)
  {
    // TODO RG initialize connection to motor board  
//    if ((serialFd = serialOpen (device, 115200)) < 0)
//    {
//      fprintf (stderr, "%s: Unable to open serial port\n", argv [0]) ;
//      exit (EXIT_FAILURE) ;
//    }
//
//    usleep (1000) ;
//
//    while (serialDataAvail (serialFd))
//      (void)serialGetchar (serialFd) ;

    serialPutchar (serialFd, '@') ;

    then = getMillis () + 5000 ;
    while (getMillis () < then)
      if ((ch = serialGetchar (serialFd)) == '@')
	break ;

    if (getMillis () >= then)
    {
      fprintf (stderr, "%s: Can't establish serial comms\n", argv [0]) ;
      exit (EXIT_FAILURE) ;
    }

// Send microseteps to the Arduino

    serialPutchar (serialFd, 'm') ;
    serialPutchar (serialFd, uSteps + '0') ;
    if (serialGetchar (serialFd) != '@')
    {
      fprintf (stderr, "%s: Arduino didn't ack. microstep command.\n", argv [0]) ;
      exit (EXIT_FAILURE) ;
    }

// Send slice thickness to Arduino

    serialPrintf (serialFd, "l%04d", sliceThickness) ;
    if (serialGetchar (serialFd) != '@')
    {
      fprintf (stderr, "%s: Arduino didn't ack. thickness command.\n", argv [0]) ;
      exit (EXIT_FAILURE) ;
    }

  }

  screenClear();
  processImages(argv [0], filenameTemplate);

  return 0 ;
}


