//  File:   FrontPanel.h
//  Defines the front panel UI device, with its OLED display & LED ring
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

#ifndef FRONTPANEL_H
#define	FRONTPANEL_H

#include <map>

#include <PrinterStatus.h>
#include <Screen.h>
#include <ScreenBuilder.h>
#include "EventType.h"
#include "ICallback.h"
#include "Settings.h"

class I_I2C_Device;

class FrontPanel : public ICallback, public IDisplay
{
public:
    FrontPanel(const I_I2C_Device& i2cDevice);
    ~FrontPanel();
    void SetAwakeTime(int minutes);

protected:
    void ClearLEDs();
    void ShowLEDs(int numLEDs);
    void ClearScreen();
    void ShowText(Alignment align, unsigned char x, unsigned char y, 
             unsigned char size, int color, std::string text);
    virtual void AnimateLEDs(int animationNum);
    void Reset();

private:
    virtual void Callback(EventType eventType, const EventData& data);
    void ShowStatus(const PrinterStatus& ps); 
    void BuildScreens();
    bool IsReady();
    void* ShowScreen(Screen* pScreen, PrinterStatus* pPS);        
    void AwaitThreadComplete();
    void SendCommand(unsigned char* buf, int len, bool awaitReady = true);
    
    static void* ThreadHelper(void *context);

    std::map<PrinterStatusKey, Screen*> _screens;
    pthread_t _showScreenThread;
    const I_I2C_Device& _i2cDevice;
    Settings& _settings;
    std::string _lastUserName;
    std::string _lastJobName;
};

// Aggregates a FrontPanel, a Screen, and PrinterStatus, 
// for passing in to a thread that handles drawing the screen
class FrontPanelScreen
{
public:
    FrontPanelScreen(FrontPanel* pFrontPanel, 
                     const PrinterStatus& ps, 
                     Screen* pScreen);
    FrontPanel* _pFrontPanel;
    PrinterStatus _PS;
    Screen* _pScreen;
};

#endif    // FRONTPANEL_H

