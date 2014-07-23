/* 
 * File:   Screen.cpp
 * Author: Richard Greene
 * 
 * Classes for handling what gets shown on the front panel displays.
 * 
 * Created on July 21, 2014, 4:58 PM
 */

#include <Screen.h>
#include <Hardware.h>

/// Constructor for a line of text that can be displayed on the screen, 
/// with the given alignment, position, size, and color. 
ScreenLine::ScreenLine(Alignment align, unsigned char x, unsigned char y, 
                       unsigned char size, int color, const char* text,
                       bool isReplaceable) :
_align(align),
_x(x),
_y(y),
_size(size),
_color(color),
_text(text),
_isReplaceable(isReplaceable)
{ 
}
 
/// TODO: define, perh just use sprintf format strings instead, and varg list?
void ScreenLine::Replace(const char* placeholder, const char* replacement)
{
    
}

/// Draw the line of text on a display.
void ScreenLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _text);
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

/// Rerturn a pointer to the first ScreenLine that contains replaceable text.
ScreenLine* ScreenText::GetReplaceable()
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        if((*it)->IsReplaceable())
            return *it;
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

/// Draw a screen with unchanging text
void Screen::Draw(IDisplay* pDisplay)
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

/// Overrides base type to insert the job name in the screen 
void JobNameScreen::Draw(IDisplay* pDisplay)
{
    // look for the ScreenLine with replaceable text
    ScreenLine* jobNameLine = _pScreenText->GetReplaceable();
    // insert the job name 
    jobNameLine->Replace(NULL, "MyJobName");
    
    Screen::Draw(pDisplay);
}