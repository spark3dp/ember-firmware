/* 
 * File:   FrontPanel.h
 * Author: Richard Greene
 * 
 * Defines the front panel UI device, with its buttons and displays
 * 
 * Created on April 15, 2014, 12:43 PM
 */

#ifndef FRONTPANEL_H
#define	FRONTPANEL_H

#include<map>

#include <I2C_Device.h>
#include <Event.h>
#include <PrinterStatus.h>
#include <Screen.h>
#include <ScreenBuilder.h>

/// Defines a front panel as an I2C device 
class FrontPanel: public I2C_Device, public ICallback, public IDisplay
{
public:
    FrontPanel(unsigned char slaveAddress);
    ~FrontPanel();
    
protected:
    // don't allow construction without specifying a slave address
    FrontPanel() {} 
    void ClearLEDs();
    void ShowLED(int ledNum);
    void ClearScreen();
    void ShowText(Alignment align, unsigned char x, unsigned char y, 
             unsigned char size, int color, const char* text);
    virtual void AnimateLEDs(int animationNum);

    private:
        virtual void Callback(EventType eventType, void* data);
        void ShowStatus(PrinterStatus* pPS); 
        void BuildScreens();
        bool IsReady();
        std::map<ScreenKey, Screen*> _screens;
};

#endif	/* FRONTPANEL_H */

