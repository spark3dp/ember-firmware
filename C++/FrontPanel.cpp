/* 
 * File:   FrontPanel.cpp
 * Author: Richard Greene
 * 
 * Implements the front panel UI device, with its buttons and displays
 * 
 * Created on April 15, 2014, 12:43 PM
 */

#include <iostream>  // for debug only

#include <FrontPanel.h>
#include <Hardware.h>
#include <ScreenBuilder.h>
#include <Logger.h>

/// Public constructor, base class opens I2C connection and sets slave address
FrontPanel::FrontPanel(unsigned char slaveAddress) :
I2C_Device(slaveAddress)
{
    // just leave the logo showing
//    // turn on the OLED display 
//    unsigned char cmdBuf[4] = {CMD_START, 2, CMD_OLED, CMD_OLED_ON};
//    Write(UI_COMMAND, cmdBuf, 4);
//    
//    ClearScreen();
    
    // clear LEDs
    AnimateLEDs(0);
    ClearLEDs();
    
    ScreenBuilder::BuildScreens(_screens);
}

/// Base class closes connection to the device
FrontPanel::~FrontPanel() 
{
    // delete all the screens
    for (std::map<std::string, Screen*>::iterator it = _screens.begin(); 
                                                  it != _screens.end(); ++it)
    {
        delete it->second;
    }
}    

/// Handles events forwarded by the event handler
void FrontPanel::Callback(EventType eventType, void* data)
{
    switch(eventType)
    {
        case PrinterStatusUpdate:
            ShowStatus((PrinterStatus*)data); 
            break;

        default:
            HandleImpossibleCase(eventType);
            break;
    }
}

/// Updates the front panel displays, based on printer status
void FrontPanel::ShowStatus(PrinterStatus* pPS)
{
//    if(pPS->_currentLayer != 0)
//    {
//        // we're currently printing, so
//        // update the bar graph with % completed
//        double pctComplete = (pPS->_currentLayer - 1) * 100.0 / pPS->_numLayers;
//        
//        // and update the OLED display with % completed and remaining time
//        int hrs = pPS->_estimatedSecondsRemaining / 3600;
//        int min = (pPS->_estimatedSecondsRemaining - (hrs * 3600)) / 60;
//        int sec = pPS->_estimatedSecondsRemaining - (hrs * 3600) - min * 60;
//        
//        
//        if(pPS->_change == Entering)
//        {   
//            if(strcmp(pPS->_state, "Exposing") == 0)
//            {
//                ClearScreen();
//                char pctMsg[20];
//                sprintf(pctMsg,"%d:%02d", hrs, min);
//                ShowText(Center, 64, 50, 2, 0xFFFF, pctMsg);
//                
//                ShowLED((int) (pctComplete * 21.0 / 100.0 + 0.5));
//            }
//            else if(strcmp(pPS->_state, "Separating") != 0)
//            {
//                ClearScreen();
//                ShowText(Center, 64, 30, 1, 0xFFFF, pPS->_state);
//            }
//        }
//    }
//    else
    {
        // based on pPS->_state, update the OLED display
        if(pPS->_change != Leaving)
        {
            // display the screen for this state and sub-state
            std::string key = pPS->_state;
            key += "_";
            key += pPS->_UISubState;
            
            if(_screens.count(key) < 1)
                key = "UNKNOWN";
            
            // here we assume we don't need to check readiness before each 
            // command, because we're never sending more than 300 bytes of
            // commands + data per screen (and the UI board has a 300 byte
            // command buffer))
            if(_screens[key] != NULL  && IsReady())
            { 
                ClearScreen();
                _screens[key]->Draw(this, pPS);
            }
        }
    }
}

/// Illuminate the given LED (first turning off all LEDs if given 0).
void FrontPanel::ShowLED(int ledNum)
{
    if(ledNum == 0)
    {     
        // stop any animation in  progress
        AnimateLEDs(0);
        // and turn all the LEDs off
        ClearLEDs();
    }
    
#ifdef DEBUG
 //   std::cout << "About to light LED # " << ledNum << std::endl;
#endif     

    // turn on the given LED to full intensity
    unsigned char cmdBuf[8] = {CMD_START, 5, CMD_RING, CMD_RING_LED, ledNum, 
                               0xFF, 0xFF, CMD_END};
    Write(UI_COMMAND, cmdBuf, 8);
}

/// Turn off all the LEDs.
void FrontPanel::ClearLEDs()
{
    unsigned char cmdBuf[7] = {CMD_START, 4, CMD_RING, CMD_RING_LEDS, 0, 0, 
                               CMD_END};
    Write(UI_COMMAND, cmdBuf, 7);  
}

/// Show an LED ring animation.
void FrontPanel::AnimateLEDs(int animationNum)
{
#ifdef DEBUG
    std::cout << "LED animation #" << animationNum << std::endl;
#endif
    
    unsigned char cmdBuf[6] = {CMD_START, 3, CMD_RING, CMD_RING_SEQUENCE, 
                               (unsigned char)animationNum, CMD_END};
    Write(UI_COMMAND, cmdBuf, 6);
}

void FrontPanel::ClearScreen()
{
    unsigned char cmdBuf[5] = {CMD_START, 2, CMD_OLED, CMD_OLED_CLEAR, CMD_END};
    Write(UI_COMMAND, cmdBuf, 5);
}

/// Show on line of text on the OLED display, using its location, alignment, 
/// size, and color.
void FrontPanel::ShowText(Alignment align, unsigned char x, unsigned char y, 
                          unsigned char size, int color, const char* text)
{
#ifdef DEBUG
//    std::cout << "Showing text: " << text << std::endl;
#endif    
    // determine the command to use, based on the alignment
    unsigned char cmd = CMD_OLED_SETTEXT;
    if(align == Center)
        cmd = CMD_OLED_CENTERTEXT;
    else if(align == Right)
        cmd = CMD_OLED_RIGHTTEXT;
    
    int textLen = strlen(text);
    if(textLen > MAX_OLED_STRING_LEN)
    {
        LOGGER.HandleError(FRONT_PANEL_STRING_TOO_LONG, false, NULL, textLen);  
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
    memcpy(cmdBuf + 10, text, textLen);
    cmdBuf[10 + textLen] = CMD_END;
    Write(UI_COMMAND, cmdBuf, 11 + textLen);
}

#define POLL_INTERVAL_MSEC (100)
#define MAX_WAIT_TIME_SEC  (10)
#define MAX_TRIES   (MAX_WAIT_TIME_SEC * 1000 / POLL_INTERVAL_MSEC) 

/// Wait until the front panel board is ready to handle commands.
bool FrontPanel::IsReady()
{
    
    bool ready = false;
    int tries = 0;
    while(tries < MAX_TRIES)
    {
        // read the I2C register to see if the board is ready to 
        // receive new commands
        unsigned char status = Read(UI_STATUS);
        // TODO: handle the case where we find (and therefore consume) button events!        
        if((status & UI_BOARD_BUSY) == 0)
        {
            ready = true;
            break;
        }        
        
        usleep(POLL_INTERVAL_MSEC * 1000);
        tries++;
    }
    
#ifdef DEBUG
    std::cout << "Polled front panel readiness " << (tries + 1) << " times" << std::endl; 
#endif   
    
    if(!ready)
        LOGGER.HandleError(FRONT_PANEL_NOT_READY); 

    return ready;
}