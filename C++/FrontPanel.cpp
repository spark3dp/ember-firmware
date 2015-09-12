//  File:   FrontPanel.cpp
//  Implements the front panel UI device, with its OLED display & LED ring
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Drew Beller
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

#include <iostream>  // for debug only

#include <FrontPanel.h>
#include <Hardware.h>
#include <Logger.h>

// Public constructor, base class opens I2C connection and sets slave address
FrontPanel::FrontPanel(unsigned char slaveAddress, int port) :
I2C_Device(slaveAddress, port),
_showScreenThread(0)
{
    // don't clear the OLED display here, just leave the logo showing

    // clear LEDs
    AnimateLEDs(0);
    ClearLEDs();

    ScreenBuilder::BuildScreens(_screens);
}

// Base class closes connection to the device
FrontPanel::~FrontPanel() 
{
    // make sure a display thread isn't still running
    AwaitThreadComplete();

    // delete all the screens
    for (std::map<int, Screen*>::iterator it = _screens.begin(); 
                                          it != _screens.end(); ++it)
    {
        delete it->second;
    }
}    

// Handles events forwarded by the event handler
void FrontPanel::Callback(EventType eventType, const EventData& data)
{
    switch(eventType)
    {
        case PrinterStatusUpdate:
            ShowStatus(data.Get<PrinterStatus>()); 
            break;

        default:
            LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(UnexpectedEvent), eventType);
            break;
    }
}

// Updates the front panel displays, based on printer status
void FrontPanel::ShowStatus(const PrinterStatus& ps)
{
    if (ps._change != Leaving)
    {
        // display the screen for this state and sub-state
        PrinterStatusKey key = PS_KEY(ps._state, ps._UISubState);

        if (ps._state == PrintingLayerState)
        {
            // force the display of the remaining print time 
            // whenever we enter or re-enter the PrintingLayer state
            // e.g. after door closed or resuming from pause
            _forceDisplay = true;   
        }
        
        if (_screens.count(key) < 1)
        {            
            std::cout << "Unknown screen for state: " << STATE_NAME(ps._state) 
                      << ", substate: " << SUBSTATE_NAME(ps._UISubState) 
                      << std::endl;
            
            key = UNKNOWN_SCREEN_KEY;
        }
        Screen* pScreen = _screens[key];
        if (pScreen != NULL)
        {
            // make sure a display thread isn't already running
            AwaitThreadComplete();
            
            // display the selected screen in a separate thread, to
            // avoid blocking here
            FrontPanelScreen* pFPS = new FrontPanelScreen(this, ps, pScreen);
            pthread_create(&_showScreenThread, NULL, &ThreadHelper, pFPS);  
        }
    }
}

// Wait for any screen drawing to be completed
void FrontPanel::AwaitThreadComplete()
{
    if (_showScreenThread != 0)
    {
        void *result;
        pthread_join(_showScreenThread, &result);
    }    
}


// Thread helper function that calls the actual screen drawing routine
void* FrontPanel::ThreadHelper(void *context)
{
    FrontPanelScreen* fps =  (FrontPanelScreen*)context; 
    fps->_pFrontPanel->ShowScreen(fps->_pScreen, &(fps->_PS));
    delete fps;
    pthread_exit(NULL);
}

// Display the selected screen 
void* FrontPanel::ShowScreen(Screen* pScreen, PrinterStatus* pPS)
{
    // no need to display null screens,
    if (pScreen != NULL)
    { 
        if (pScreen->NeedsLEDClear())
        {
            AnimateLEDs(0);
            ClearLEDs();
        }
        if (pScreen->NeedsScreenClear())
            ClearScreen();

        pScreen->Draw(this, pPS);
    }  
    return NULL;
}

// Illuminate the given number of LEDs 
void FrontPanel::ShowLEDs(int numLEDs)
{   
    if (numLEDs < 0 || numLEDs > NUM_LEDS_IN_RING)
        return; // invalid number of LEDs to light
    
    for(int i = 0; i < NUM_LEDS_IN_RING; i++)
    {
        // turn on the given number of LEDs (+1) to full intensity, 
        // and turn the rest off
        unsigned char color = (i <= numLEDs) ? 0xFF : 0;
        unsigned char cmdBuf[8] = {CMD_START, 5, CMD_RING, CMD_RING_LED, i, 
                                   color, color, CMD_END};
        // only do a ready wait on first call
        SendCommand(cmdBuf, 8, i == 0);
        if (i < NUM_LEDS_IN_RING - 1)
            usleep(10);  // wait 10us to avoid having LED #3 not turn on
    }
}

// Turn off all the LEDs.
void FrontPanel::ClearLEDs()
{
    unsigned char cmdBuf[7] = {CMD_START, 4, CMD_RING, CMD_RING_LEDS, 0, 0, 
                               CMD_END};
    SendCommand(cmdBuf, 7);  
}

// Show an LED ring animation.
void FrontPanel::AnimateLEDs(int animationNum)
{    
    unsigned char cmdBuf[6] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 
                               (unsigned char)animationNum, CMD_END};
    SendCommand(cmdBuf, 6);
}

// Clear the OLED display
void FrontPanel::ClearScreen()
{    
    unsigned char cmdBuf[5] = {CMD_START, 2, CMD_OLED, CMD_OLED_CLEAR, CMD_END};
    SendCommand(cmdBuf, 5);
}

// Software Reset on the FrontPanel
void FrontPanel::Reset()
{    
    unsigned char cmdBuf[4] = {CMD_START, 2, CMD_RESET, CMD_END};

    SendCommand(cmdBuf, 4);
}

// Show on line of text on the OLED display, using its location, alignment, 
// size, and color.
void FrontPanel::ShowText(Alignment align, unsigned char x, unsigned char y, 
                          unsigned char size, int color, std::string text)
{    
    // determine the command to use, based on the alignment
    unsigned char cmd = CMD_OLED_SETTEXT;
    if (align == Center)
        cmd = CMD_OLED_CENTERTEXT;
    else if (align == Right)
        cmd = CMD_OLED_RIGHTTEXT;
    
    int textLen = text.length();
    if (textLen > MAX_OLED_STRING_LEN)
    {
        LOGGER.HandleError(LongFrontPanelString, false, NULL, textLen);  
        // truncate text to prevent overrunning the front panel's I2C buffer 
        textLen = MAX_OLED_STRING_LEN;
    }
    
    // the command structure is:
    // [CMD_START][FRAME LENGTH][CMD_OLED][CMD_OLED_xxxTEXT][X BYTE][Y BYTE]
    // [SIZE BYTE] [HI COLOR BYTE][LO COLOR BYTE][TEXT LENGTH][TEXT BYTES] ...
    // [CMD_END]
    unsigned char cmdBuf[35] = 
        {CMD_START, 8 + textLen, CMD_OLED, cmd, x, y, size, 
         (unsigned char)((color & 0xFF00) >> 8), (unsigned char)(color & 0xFF), 
         textLen};
    memcpy(cmdBuf + 10, text.c_str(), textLen);
    cmdBuf[10 + textLen] = CMD_END;
    SendCommand(cmdBuf, 11 + textLen);
}

#define POLL_INTERVAL_MSEC (10)
#define MAX_WAIT_TIME_SEC  (10)
#define MAX_READY_TRIES   (MAX_WAIT_TIME_SEC * 1000 / POLL_INTERVAL_MSEC) 

// Wait until the front panel is ready to handle commands.
bool FrontPanel::IsReady()
{
    bool ready = false;
    int tries = 0;
    while(tries < MAX_READY_TRIES)
    {
        // read the I2C register to see if the front panel is ready to 
        // receive new commands
        unsigned char status = Read(DISPLAY_STATUS);

        if ((status & FP_BUSY) == 0)
        {
            ready = true;
            break;
        }        
        
        usleep(POLL_INTERVAL_MSEC * 1000);
        tries++;
    }
       
    if (!ready)
        LOGGER.HandleError(FrontPanelNotReady); 

    return ready;
}

// Send a command to the front panel, checking readiness first and retrying
// on I2C write failure.
void FrontPanel::SendCommand(unsigned char* buf, int len, bool awaitReady)
{
    if (awaitReady)
        IsReady();

    int tries = 0;
    while(tries++ < MAX_I2C_CMD_TRIES && !Write(FP_COMMAND, buf, len))
        ; 
}

// Set the time after which the screen goes to sleep (to extend the lifetime
// of the OLED display), if there's been no button presses or commands.
// Valid values are 1-255 minutes, or 0 to disable screen saving.
void FrontPanel::SetAwakeTime(int minutes)
{    
    unsigned char cmdBuf[5] = {CMD_START, 2, CMD_SLEEP, (unsigned char)minutes,
                                CMD_END};
    SendCommand(cmdBuf, 5);
}


// Constructor
FrontPanelScreen::FrontPanelScreen(FrontPanel* pFrontPanel, 
                                   const PrinterStatus& ps, 
                                   Screen* pScreen) :
_pFrontPanel(pFrontPanel),
_PS(ps),
_pScreen(pScreen)          
{
}
