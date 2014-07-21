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

#include <I2C_Device.h>
#include <Event.h>
#include <PrinterStatus.h>

/// Defines a front panel as an I2C device 
class FrontPanel: public I2C_Device, public ICallback
{
public:
    FrontPanel(unsigned char slaveAddress);
    ~FrontPanel();
    
protected:
    // don't allow construction without specifying a slave address
    FrontPanel() {} 
    void ClearLEDs();
    void ShowLED(int ledNum);
    void AnimateLEDRing(unsigned char n);
    void ShowText(unsigned char x, unsigned char y, unsigned char size, 
                  int color, const char* text);
    void ClearScreen();
    
    private:
        virtual void Callback(EventType eventType, void* data);
        void ShowStatus(PrinterStatus* pPS); 
};

#endif	/* FRONTPANEL_H */

