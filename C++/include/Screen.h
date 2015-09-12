//  File:   Screen.h
//  Defines classes for handling what gets shown on the front panel displays
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

#ifndef SCREEN_H
#define	SCREEN_H

#include <vector>
#include <string>

#include <PrinterStatus.h>


// The options for aligning text on a screen.
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

// ABC for a class that knows how to display a line of text and to show LED 
// ring animations.
class IDisplay
{
public: 
    virtual void ShowText(Alignment align, unsigned char x, unsigned char y, 
             unsigned char size, int color, std::string text) = 0;
    virtual void AnimateLEDs(int animationNum) = 0;
    virtual void ShowLEDs(int numLEDs) = 0;
    bool _forceDisplay;
};

// ABC for a class that knows how to draw itself on an IDisplay.
class IDrawable
{
public: 
    virtual void Draw(IDisplay* pDisplay) = 0;
};

class ScreenLine : IDrawable
{
public:
    ScreenLine(Alignment align, unsigned char x, unsigned char y, 
               unsigned char size, int color, std::string text);
    virtual void Draw(IDisplay* pDisplay);

protected: 
    Alignment _align;
    unsigned char _x;
    unsigned char _y;
    unsigned char _size;
    int _color;
    std::string _text; 
    
private:
    ScreenLine() {} // don't allow default construction  
};

class ReplaceableLine : public ScreenLine
{
public:
    ReplaceableLine(Alignment align, unsigned char x, unsigned char y, 
                    unsigned char size, int color, std::string text);
    void ReplaceWith(std::string replacement);
    void Draw(IDisplay* pDisplay);
    
protected:
    std::string _replacedText;
};

class ScreenText : public IDrawable
{
public:
    void Add(ScreenLine* pScreenLine);
    void Draw(IDisplay* pDisplay);
    ~ScreenText();
    ReplaceableLine* GetReplaceable(int n = 1);
    
private:
    std::vector<ScreenLine*> _pScreenLines;
};


class Screen 
{
public:
    Screen(ScreenText* pScreenText, int ledAnimation, 
           bool needsScreenClear = true, bool needsLEDClear = true);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);
    ~Screen();
    bool NeedsScreenClear() {return _needsScreenClear; }
    bool NeedsLEDClear() {return _needsLEDClear; }
    
protected:
    ScreenText* _pScreenText;
    bool _needsScreenClear;    
    bool _needsLEDClear;    
    
private:
    int _LEDAnimation;
};

class JobNameScreen : public Screen
{
public:
    JobNameScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);
};

class ErrorScreen : public Screen
{
public:
    ErrorScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

class PrintStatusScreen : public Screen
{
public:
    PrintStatusScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus); 
    
private:
    std::string _previousTime;    
};

class RegistrationScreen : public Screen
{
public:
    RegistrationScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

class UnknownScreen : public Screen
{
public:
    UnknownScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

class SysInfoScreen : public Screen
{
public:
    SysInfoScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

class USBFileFoundScreen : public Screen
{
public:
    USBFileFoundScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

class USBErrorScreen : public Screen
{
public:
    USBErrorScreen(ScreenText* pScreenText, int ledAnimation);
    virtual void Draw(IDisplay* pDisplay, PrinterStatus* pStatus);    
};

#endif    // SCREEN_H

