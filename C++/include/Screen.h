/* 
 * File:   Screen.h
 * Author: Richard Greene
 *
 * Classes for handling what gets shown on the front panel displays.
 * 
 * Created on July 21, 2014, 4:58 PM
 */

#ifndef SCREEN_H
#define	SCREEN_H

#include <vector>

#include <PrinterStatus.h>


/// The options for aligning text on a screen.
enum Alignment
{
    // Uninitialized alignment
    UndefinedAlignment = 0,
    
    // The X position defines the left edge of the text.
    Left,
    
    // The X position defines the center position of the text.
    Center,
    
    // The X position defines the right edge of the text.
    Right      
};

/// ABC for a class that knows how to display a line of text and to show LED 
/// animations.
class IDisplay
{
public: 
    virtual void ShowText(Alignment align, unsigned char x, unsigned char y, 
             unsigned char size, int color, const char* text) = 0;
    virtual void AnimateLEDs(int animationNum) = 0;
};

/// ABC for a class that knows how to draw itself on an IDisplay.
class IDrawable
{
public: 
    virtual void Draw(IDisplay* pDisplay) = 0;
};

class ScreenLine : IDrawable
{
public:
    ScreenLine(Alignment align, unsigned char x, unsigned char y, 
               unsigned char size, int color, const char* text);
    void Replace(const char* placeholder, const char* replacement);
    void Draw(IDisplay* pDisplay);    
      
private:
    ScreenLine() {} // don't allow default construction
    Alignment _align;
    unsigned char _x;
    unsigned char _y;
    unsigned char _size;
    int _color;
    const char* _text;    
};

class ScreenText : public IDrawable
{
public:
    void Add(ScreenLine screenLine);
    void Draw(IDisplay* pDisplay);
    
private:
    std::vector<ScreenLine> _screenLines;
};


class Screen : public IDrawable
{
public:
    Screen(ScreenText text, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay);
    
private:
    Screen(){} // don't allow default construction 
    ScreenText _text;
    int _LEDAnimation;
};

#endif	/* SCREEN_H */

