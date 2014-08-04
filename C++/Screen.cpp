/* 
 * File:   Screen.cpp
 * Author: Richard Greene
 * 
 * Classes for handling what gets shown on the front panel displays.
 * 
 * Created on July 21, 2014, 4:58 PM
 */

#include <string.h>
#include <stdio.h>

#include <Screen.h>
#include <Hardware.h>
#include <Settings.h>
#include <utils.h>

/// Constructor for a line of text that can be displayed on the screen, 
/// with the given alignment, position, size, and color. 
ScreenLine::ScreenLine(Alignment align, unsigned char x, unsigned char y, 
                       unsigned char size, int color, const char* text) :
_align(align),
_x(x),
_y(y),
_size(size),
_color(color),
_text(text)
{ 
}

/// Draw the line of text on a display.
void ScreenLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _text.c_str());
}

// Constructor, just calls base type
ReplaceableLine::ReplaceableLine(Alignment align, unsigned char x, 
                                 unsigned char y, unsigned char size, int color,
                                 const char* text) :
ScreenLine(align, x, y, size, color, text)                                 
{
}

/// Replace the placeholder text (or if it's null, use sprintf formatting)
void ReplaceableLine::Replace(const char* placeholder, std::string replacement)
{
    if(placeholder == NULL)
    {
        int len = _text.length() + replacement.length();
        char buf[len];
        sprintf(buf, _text.c_str(), replacement.c_str());
        _replacedText = buf;
    }
}

/// Draw the replaced line of text on a display.
void ReplaceableLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _replacedText.c_str());
}

/// Destructor deletes the contained ScreenLines.
ScreenText::~ScreenText()
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        delete (*it);
    }  
}

/// Add a line to the screen's collection of text.
void ScreenText::Add(ScreenLine* pScreenLine)
{
    _pScreenLines.push_back(pScreenLine);
}

/// Draw the collection of text lines on a display.
void ScreenText::Draw(IDisplay* pDisplay)
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        (*it)->Draw(pDisplay);
    }  
}

/// Return a pointer to the nth ScreenLine that contains replaceable text.
ReplaceableLine* ScreenText::GetReplaceable(int n)
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        ReplaceableLine* pRL = dynamic_cast<ReplaceableLine*>(*it);
        if(pRL != NULL && 0 == --n)
            return pRL;
    }  
    return NULL;
}

/// Constructor for a screen of text plus an accompanying LED animation.  
/// (Animation 0 means no LED animation fdor this screen.))
Screen::Screen(ScreenText* pScreenText, int ledAnimation) :
_pScreenText(pScreenText),
_LEDAnimation(ledAnimation),
_needsClear(true)        
{ 
}

/// Destructor deletes contained ScreenText*.
Screen::~Screen()
{
    delete _pScreenText;
}

/// Draw a screen with unchanging text.  (PrinterStatus is unused in this 
/// base type.)
void Screen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // draw the text
    _pScreenText->Draw(pDisplay);
    
    // show the LED animation
    pDisplay->AnimateLEDs(_LEDAnimation);
}

// Constructor, just calls base type
JobNameScreen::JobNameScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

#define FIRST_NUM_CHARS (9)
#define LAST_NUM_CHARS  (5)

/// Overrides base type to insert the job name in the screen 
void JobNameScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* jobNameLine = _pScreenText->GetReplaceable();
    
    if(jobNameLine != NULL)
    {
        // get the job name
        std::string jobName = SETTINGS.GetString(JOB_NAME_SETTING);

        if(jobName.length() > MAX_OLED_STRING_LEN - 2)
        {
            // job name (plus quotes) is too long , so truncate it by taking 
            // first and last characters, separated by elipsis
            jobName = jobName.substr(0,FIRST_NUM_CHARS) + "..." + 
                      jobName.substr(jobName.length() - LAST_NUM_CHARS, 
                                     LAST_NUM_CHARS);
        }
        // insert the job name 
        jobNameLine->Replace(NULL, jobName);
    }
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
ErrorScreen::ErrorScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

/// Overrides base type to insert the error code and errno in the screen 
void ErrorScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* errorCodeLine = _pScreenText->GetReplaceable();
    
    if(errorCodeLine != NULL)
    {
        char errorCodes[20];
        sprintf(errorCodes,"%d-%d", pStatus->_errorCode, pStatus->_errno);

        // insert the error codes 
        errorCodeLine->Replace(NULL, std::string(errorCodes));
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, calls base type but doesn't want screen cleared first
PrintStatusScreen::PrintStatusScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation),
_previousTime("")
{ 
    _needsClear = false;
}

/// Overrides base type to show the print time remaining  
void PrintStatusScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* eraseLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* timeLine = _pScreenText->GetReplaceable(2);
    
    if(eraseLine != NULL && timeLine != NULL)
    {      
        // get and format the remaining time 
        int hrs = pStatus->_estimatedSecondsRemaining / 3600;
        int min = (pStatus->_estimatedSecondsRemaining - (hrs * 3600)) / 60;
        char timeRemaining[20];
        sprintf(timeRemaining,"%d:%02d", hrs, min);
        
        std::string time(timeRemaining);
        // uncomment the following line to only update the time when it changes
        // (but then on door open/close or pause/resume, no time will be shown 
        // till next change of minutes)
     //   if(_previousTime.compare(time) != 0)
        {
            // erase the time already showing
            eraseLine->Replace(NULL, _previousTime);
            // insert the remaining time
            timeLine->Replace(NULL, time);
            // and record the change
            _previousTime = time;

            Screen::Draw(pDisplay, pStatus);
            
            // show percent completion via LEDs 
            double pctComplete = (pStatus->_currentLayer - 1.0) / 
                                  pStatus->_numLayers;
            
            pDisplay->ShowLEDs((int)(NUM_LEDS_IN_RING * pctComplete + 0.5));
#ifdef DEBUG
            std::cout << "percent complete =  " << pctComplete * 100. 
                      << std::endl;
#endif           
        }
    }
    
}