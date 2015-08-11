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

#include <map>

#include <I2C_Device.h>
#include <Event.h>
#include <PrinterStatus.h>
#include <Screen.h>
#include <ScreenBuilder.h>



/// Defines a front panel as an I2C device 
class FrontPanel: public I2C_Device, public ICallback, public IDisplay
{
public:
    FrontPanel(unsigned char slaveAddress, int port);
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
    virtual void Callback(EventType eventType, void* data);
    void ShowStatus(PrinterStatus* pPS); 
    void BuildScreens();
    bool IsReady();
    std::map<PrinterStatusKey, Screen*> _screens;
    void* ShowScreen(Screen* pScreen, PrinterStatus* pPS);        
    static void* ThreadHelper(void *context);
    pthread_t _showScreenThread;
    void AwaitThreadComplete();
    void SendCommand(unsigned char* buf, int len, bool awaitReady = true);
};

/// Aggregates a FrontPanel, a Screen, and PrinterStatus, 
/// for passing in to a thread that handles drawing the screen
class FrontPanelScreen
{
public:
    FrontPanelScreen(FrontPanel* pFrontPanel, 
                     PrinterStatus& ps, 
                     Screen* pScreen);
    FrontPanel* _pFrontPanel;
    PrinterStatus _PS;
    Screen* _pScreen;
private:
    FrontPanelScreen() {}; // prevent construction without passing in members
};

#endif	/* FRONTPANEL_H */

