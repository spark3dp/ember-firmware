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
#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if(x);                                            \
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

/// Constructor for a line of text that can be displayed on the screen, 
/// with the given alignment, position, size, and color. 
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

/// Draw the line of text on a display.
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

/// Replace the placeholder text using sprintf formatting
void ReplaceableLine::ReplaceWith(std::string replacement)
{
        int len = _text.length() + replacement.length();
        char buf[len];
        sprintf(buf, _text.c_str(), replacement.c_str());
        _replacedText = buf;
}

/// Draw the replaced line of text on a display.
void ReplaceableLine::Draw(IDisplay* pDisplay)
{
    pDisplay->ShowText(_align, _x, _y, _size, _color, _replacedText);
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
/// (Animation 0 means no LED animation for this screen.))
Screen::Screen(ScreenText* pScreenText, int ledAnimation, 
               bool needsScreenClear, bool needsLEDClear) :
_pScreenText(pScreenText),
_LEDAnimation(ledAnimation),
_needsScreenClear(needsScreenClear),        
_needsLEDClear(needsLEDClear)        
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
        jobNameLine->ReplaceWith(jobName);
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
    // look for the ScreenLines with replaceable text
    ReplaceableLine* errorCodeLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* errorMsgLine = _pScreenText->GetReplaceable(2);
    
    if(errorCodeLine != NULL && errorMsgLine != NULL)
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
PrintStatusScreen::PrintStatusScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation, false, false),
_previousTime("")
{ 
}

/// Overrides base type to show the print time remaining  
void PrintStatusScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* eraseLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* timeLine = _pScreenText->GetReplaceable(2);
    
    if(eraseLine != NULL && timeLine != NULL)
    {      
        // get and format the remaining time (rounded to nearest minute))
        int roundingTime = pStatus->_estimatedSecondsRemaining + 30;
        int hrs = roundingTime / 3600;
        int min = (roundingTime - (hrs * 3600)) / 60;

        char timeRemaining[20];
        sprintf(timeRemaining,"%d:%02d", hrs, min);
        
        std::string time(timeRemaining);
        // uncomment the following line to only update the time when it changes
        // (but then on door open/close or pause/resume, no time will be shown 
        // till next change of minutes)
     //   if(_previousTime.compare(time) != 0)
        {
            // erase the time already showing
            eraseLine->ReplaceWith(_previousTime);
            // insert the remaining time
            timeLine->ReplaceWith(time);
            // and record the change
            _previousTime = time;

            Screen::Draw(pDisplay, pStatus);
            
            // show percent completion via LEDs (but only have the animation 
            // after print completion to light the last of the LEDs)
            double pctComplete = (pStatus->_currentLayer - 1.0) / 
                                  pStatus->_numLayers;
            if(pctComplete >= 0 && pctComplete <= 1 )
                pDisplay->ShowLEDs((int)((NUM_LEDS_IN_RING - 1) * pctComplete + 0.5));
#ifdef DEBUG
//            std::cout << "percent complete =  " << pctComplete * 100 
//                                                                   << std::endl;
#endif           
        }
    }
}

// Constructor, just calls base type
RegistrationScreen::RegistrationScreen(ScreenText* pScreenText, int ledAnimation) :
Screen(pScreenText, ledAnimation)
{ 
}

#define LOAD_BUF_LEN (1024)
/// Overrides base type to insert the registration URL & code in the screen 
void RegistrationScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* regURLLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* regCodeLine = _pScreenText->GetReplaceable(2);
    
    
    if(regURLLine != NULL && regCodeLine != NULL)
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

/// Screen shown when no screen has been defined for the given state and
/// UI sub-state.  Overrides base type to insert their names in the screen. 
void UnknownScreen::Draw(IDisplay* pDisplay, PrinterStatus* pStatus)
{
    // look for the ScreenLines with replaceable text
    ReplaceableLine* stateLine = _pScreenText->GetReplaceable(1);
    ReplaceableLine* substateLine = _pScreenText->GetReplaceable(2);
    
    
    if(stateLine != NULL && substateLine != NULL)
    {    
        // insert the state and substate
        stateLine->ReplaceWith(STATE_NAME(pStatus->_state));
        substateLine->ReplaceWith(SUBSTATE_NAME(pStatus->_UISubState));
    }
    
    Screen::Draw(pDisplay, pStatus);
}

    