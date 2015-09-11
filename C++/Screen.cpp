//  File:   Screen.cpp
//  Implements classes for handling what gets shown on the front panel displays
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>  
#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if (x);                                            \
  else throw std::exception();  

#include <rapidjson/reader.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>

#include <Screen.h>
#include <Hardware.h>
#include <Settings.h>
#include <utils.h>
#include <Filenames.h>
#include <MessageStrings.h>
#include <Shared.h>

using namespace rapidjson;

// reduce the max allowed length for unknown strings, to avoid wrapparound due
// to the proportional font
#define MAX_UNKNOWN_STRING_LEN   (MAX_OLED_STRING_LEN - 2)

// Constructor for a line of text that can be displayed on the screen, 
// with the given alignment, position, size, and color. 
ScreenLine::ScreenLine(Alignment align, unsigned char x, unsigned char y, 
                       unsigned char size, int color, std::string text) :
_align(align),
_x(x),
_y(y),
_size(size),
_color(color),
_text(text)
{ 
}

// Draw the line of text on a display.
void ScreenLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _text.c_str());
}

// Constructor, just calls base type
ReplaceableLine::ReplaceableLine(Alignment align, unsigned char x, 
                                 unsigned char y, unsigned char size, int color,
                                 std::string text) :
ScreenLine(align, x, y, size, color, text)                                 
{
}

// Replace the placeholder text using sprintf formatting
void ReplaceableLine::ReplaceWith(std::string replacement)
{
        int len = _text.length() + replacement.length();
        char buf[len];
        sprintf(buf, _text.c_str(), replacement.c_str());
        _replacedText = buf;
}

// Draw the replaced line of text on a display.
void ReplaceableLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _replacedText);
}

// Destructor deletes the contained ScreenLines.
ScreenText::~ScreenText()
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        delete (*it);
    }  
}

// Add a line to the screen's collection of text.
void ScreenText::Add(ScreenLine* pScreenLine)
{
    _pScreenLines.push_back(pScreenLine);
}

// Draw the collection of text lines on a display.
void ScreenText::Draw(IDisplay* pDisplay)
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        (*it)->Draw(pDisplay);
    }  
}

// Return a pointer to the nth ScreenLine that contains replaceable text.
ReplaceableLine* ScreenText::GetReplaceable(int n)
{
    for (std::vector<ScreenLine*>::iterator it = _pScreenLines.begin(); 
                                            it != _pScreenLines.end(); ++it)
    {
        ReplaceableLine* pRL = dynamic_cast<ReplaceableLine*>(*it);
        if (pRL != NULL && 0 == --n)
            return pRL;
    }  
    return NULL;
}

// Constructor for a screen of text plus an accompanying LED animation.  
// (Animation 0 means no LED animation for this screen.))
Screen::Screen(ScreenText* pScreenText, int ledAnimation, 
               bool needsScreenClear, bool needsLEDClear) :
_pScreenText(pScreenText),
_LEDAnimation(ledAnimation),
_needsScreenClear(needsScreenClear),        
_needsLEDClear(needsLEDClear)        
{ 
}

// Destructor deletes contained ScreenText*.
Screen::~Screen()
{
    delete _pScreenText;
}

// Draw a screen with unchanging text.  (PrinterStatus is unused in this 
// base type.)
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

// Overrides base type to insert the job name in the screen 
void JobNameScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* jobNameLine = _pScreenText->GetReplaceable();
    
    if (jobNameLine != NULL)
    {
        // get the job name
        std::string jobName = SETTINGS.GetString(JOB_NAME_SETTING);

        if (jobName.length() > MAX_UNKNOWN_STRING_LEN)
        {
            // job name is too long, so truncate it by taking 
            // first and last characters, separated by ellipsis
            jobName = jobName.substr(0,FIRST_NUM_CHARS) + "..." + 
                      jobName.substr(jobName.length() - LAST_NUM_CHARS, 
                                     LAST_NUM_CHARS);
        }
        // insert the job name 
        jobNameLine->ReplaceWith(jobName);
    }
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
ErrorScreen::ErrorScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

// Overrides base type to insert the error code and errno in the screen 
void ErrorScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* errorCodeLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* errorMsgLine = _pScreenText->GetReplaceable(2);
    
    if (errorCodeLine != NULL && errorMsgLine != NULL)
    {
        char errorCodes[20];
        sprintf(errorCodes,"%d-%d", pStatus->_errorCode, pStatus->_errno);

        // insert the error codes 
        errorCodeLine->ReplaceWith(errorCodes);
        
        // get the short error message (if any) for the code)
        errorMsgLine->ReplaceWith(SHORT_ERR_MSG(pStatus->_errorCode));
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, calls base type but doesn't want screen cleared first
PrintStatusScreen::PrintStatusScreen(ScreenText* pScreenText, 
                                     int ledAnimation) :
Screen(pScreenText, ledAnimation, false, false),
_previousTime("")
{ 
}

// Overrides base type to show the print time remaining and percent completion 
void PrintStatusScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* eraseLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* timeLine = _pScreenText->GetReplaceable(2);
    
    if (eraseLine != NULL && timeLine != NULL)
    {      
        // get and format the remaining time (rounded to nearest minute))
        int roundingTime = pStatus->_estimatedSecondsRemaining + 30;
        int hrs = roundingTime / 3600;
        int min = (roundingTime - (hrs * 3600)) / 60;

        char timeRemaining[20];
        sprintf(timeRemaining,"%d:%02d", hrs, min);
        
        std::string time(timeRemaining);
        
        // only update the time when it changes, or we need to force its
        // redisplay (e.g. after closing door or resuming from pause)
        if (_previousTime.compare(time) != 0 || pDisplay->_forceDisplay)
        {
            pDisplay->_forceDisplay = false;
            // erase the time already showing
            eraseLine->ReplaceWith(_previousTime);
            // insert the remaining time
            timeLine->ReplaceWith(time);
            // and record the change
            _previousTime = time;

            Screen::Draw(pDisplay, pStatus);          
        }      
    }
    
    // show percent completion via the ring of LEDs (but only light the last of 
    // the LEDs when the print completion animation is shown)
    double pctComplete = (pStatus->_currentLayer - 1.0) / pStatus->_numLayers;
    if (pctComplete >= 0 && pctComplete <= 1)
        pDisplay->ShowLEDs((int)((NUM_LEDS_IN_RING - 1) * pctComplete + 0.5));          
}

// Constructor, just calls base type
RegistrationScreen::RegistrationScreen(ScreenText* pScreenText, 
                                       int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

#define LOAD_BUF_LEN (1024)
// Overrides base type to insert the registration URL & code in the screen 
void RegistrationScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* regURLLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* regCodeLine = _pScreenText->GetReplaceable(2);
    
    
    if (regURLLine != NULL && regCodeLine != NULL)
    {
        // get registration code & URL from file created by web client
        const char* regURL = UNKNOWN_REGISTRATION_URL;
        const char* regCode = UNKNOWN_REGISTRATION_CODE;
        try
        {
            FILE* pFile = fopen(PRIMARY_REGISTRATION_INFO_FILE, "r");
            char buf[LOAD_BUF_LEN];
            FileReadStream frs1(pFile, buf, LOAD_BUF_LEN);
            // first parse into a temporary doc, for validation
            Document doc;
            doc.ParseStream(frs1);

            // make sure the file is valid
            RAPIDJSON_ASSERT(doc.IsObject() &&
                             doc.HasMember(REGISTRATION_URL_KEY) &&
                             doc.HasMember(REGISTRATION_CODE_KEY))
                    
            regURL = doc[REGISTRATION_URL_KEY].GetString();
            regCode = doc[REGISTRATION_CODE_KEY].GetString();
            
            fclose(pFile);
        }
        catch(std::exception)
        {
            LOGGER.HandleError(CantReadRegistrationInfo, false, 
                               PRIMARY_REGISTRATION_INFO_FILE);
        }
        
        // insert the URL & registration code
        regURLLine->ReplaceWith(regURL);
        regCodeLine->ReplaceWith(regCode);
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
UnknownScreen::UnknownScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

// Screen shown when no screen has been defined for the given state and
// UI sub-state.  Overrides base type to insert their names in the screen. 
void UnknownScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* stateLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* substateLine = _pScreenText->GetReplaceable(2);
    
    
    if (stateLine != NULL && substateLine != NULL)
    {    
        // insert the state and substate
        stateLine->ReplaceWith(STATE_NAME(pStatus->_state));
        substateLine->ReplaceWith(SUBSTATE_NAME(pStatus->_UISubState));
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
SysInfoScreen::SysInfoScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

// Screen shown to display system information, such as the firmware version and
// IP address. 
void SysInfoScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* ipAddressLine = _pScreenText->GetReplaceable(1);
    
    if (ipAddressLine != NULL)
    {    
        // insert the IP address
        ipAddressLine->ReplaceWith(GetIPAddress().c_str());
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
USBFileFoundScreen::USBFileFoundScreen(ScreenText* pScreenText, 
                                       int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

// Screen shown when a print data file was found on a USB drive. 
void USBFileFoundScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* line1 = _pScreenText->GetReplaceable(1);
    ReplaceableLine* line2 = _pScreenText->GetReplaceable(2);
    ReplaceableLine* line3 = _pScreenText->GetReplaceable(3);
    
    if (line1 != NULL  && line2 != NULL && line3 != NULL)
    {  
        std::string fileName = pStatus->_usbDriveFileName;
        
        int maxLen = MAX_UNKNOWN_STRING_LEN;
        int endLen = LAST_NUM_CHARS + MAX_UNKNOWN_STRING_LEN;
        
        if (fileName.length() > 3 * maxLen)
        {
            // file name is too long, so truncate it by taking 
            // first and last characters, separated by ellipsis
            fileName = fileName.substr(0,FIRST_NUM_CHARS + maxLen) + "..." + 
                       fileName.substr(fileName.length() - endLen, endLen);

        }
        line1->ReplaceWith(fileName.substr(0, maxLen));
        if (fileName.length() > maxLen)
            line2->ReplaceWith(fileName.substr(maxLen, maxLen));
        else
            line2->ReplaceWith("");
        if (fileName.length() > 2 * maxLen)
            line3->ReplaceWith(fileName.substr(2 * maxLen, maxLen));
        else
            line3->ReplaceWith("");
    }
    
    Screen::Draw(pDisplay, pStatus);
}

// Constructor, just calls base type
USBErrorScreen::USBErrorScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

// Screen shown when there's a USB drive error. 
void USBErrorScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLine with replaceable text
    ReplaceableLine* dirLine = _pScreenText->GetReplaceable(1);
    
    if (dirLine != NULL)
    {    
        // insert the name of the folder in which we look for print data
        dirLine->ReplaceWith(
                                SETTINGS.GetString(USB_DRIVE_DATA_DIR).c_str());
    }
    
    Screen::Draw(pDisplay, pStatus);
}
