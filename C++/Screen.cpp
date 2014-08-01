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

/// Return a pointer to the first ScreenLine that contains replaceable text.
ReplaceableLine* ScreenText::GetReplaceable()
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        ReplaceableLine* pRL = dynamic_cast<ReplaceableLine*>(*it);
        if(pRL != NULL)
            return pRL;
    }  
    return NULL;
}

/// Constructor for a screen of text plus an accompanying LED animation.  
/// (Animation 0 means no LED animation fdor this screen.))
Screen::Screen(ScreenText* pScreenText, int ledAnimation) :
_pScreenText(pScreenText),
_LEDAnimation(ledAnimation)        
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
VersionScreen::VersionScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

/// Overrides base type to insert the current firmware version  in the screen 
void VersionScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* versionLine = _pScreenText->GetReplaceable();
    
    if(versionLine != NULL)
    {
        // insert the version number 
        versionLine->Replace(NULL, std::string(GetFirmwareVersion()));
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
        // TODO
        // get the error code & errno and format them
        std::string errorCodes = "XXX-YYYY";

        // insert the error codes 
        errorCodeLine->Replace(NULL, errorCodes);
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
PrintStatusScreen::PrintStatusScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

/// Overrides base type to insert the time remaining in the screen 
void PrintStatusScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* timeLine = _pScreenText->GetReplaceable();
    
    if(timeLine != NULL)
    {
        // get and format the remaining time 
        int hrs = pStatus->_estimatedSecondsRemaining / 3600;
        int min = (pStatus->_estimatedSecondsRemaining - (hrs * 3600)) / 60;
        char timeRemaining[20];
        sprintf(timeRemaining,"%d:%02d", hrs, min);

        // insert the remaining time
        timeLine->Replace(NULL, std::string(timeRemaining));

        // TODO: needs to show percent completion via LEDs rather than showing
        // an LED animation (don't even want a null animation)
        double pctComplete = (pStatus->_currentLayer - 1) * 100.0 / 
                              pStatus->_numLayers;
    }
    Screen::Draw(pDisplay, pStatus);
}