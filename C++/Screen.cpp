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
                       unsigned char size, int color, const char* text) :
_align(align),
_x(x),
_y(y),
_size(size),
_color(color),
_text(text)        
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

/// Add a line to the screen's collection of text.
void ScreenText::Add(ScreenLine screenLine)
{
    _screenLines.push_back(screenLine);
}

/// Draw the collection of text lines on a display.
void ScreenText::Draw(IDisplay* pDisplay)
{
    for (std::vector<ScreenLine>::iterator it = _screenLines.begin(); 
                                           it != _screenLines.end(); ++it)
    {
        it->Draw(pDisplay);
    }  
}

/// Constructor for a screen of text plus an accompanying LED animation.  
/// (Animation 0 means no LED animation fdor this screen.))
Screen::Screen(ScreenText text, int ledAnimation) :
_text(text),
_LEDAnimation(ledAnimation)        
{ 
}

void Screen::Draw(IDisplay* pDisplay)
{
    // draw the text
    _text.Draw(pDisplay);
    
    // show the LED animation
    pDisplay->AnimateLEDs(_LEDAnimation);
}
