/* 
 * File:   ScreenBuilder.cpp
 * Author: Richard Greene
 * 
 * Builds the screens shown on the front panel.
 * 
 * Created on July 23, 2014, 12:15 PM
 */

#include <ScreenBuilder.h>
#include <PrinterStatus.h>
#include <ScreenLayouts.h>

#define UNDEFINED_SCREEN_LINE1  Center, 64, 10, 1, 0xFFFF, "Screen?"
#define UNDEFINED_SCREEN_LINE2  Center, 64, 30, 1, 0xFFFF, "%s"

/// Build the screens and add them to a map keyed by PrintEngine states and UI
/// sub states.
void ScreenBuilder::BuildScreens(std::map<std::string, Screen*>& screenMap) 
{
    ScreenText* unknown = new ScreenText();
    unknown->Add(new ScreenLine(UNDEFINED_SCREEN_LINE1));
    // TODO: add the following when we can replace %s 
    // with the current state & substate
  //  unknown->Add(ScreenLine(UNDEFINED_SCREEN_LINE2));
    screenMap["UNKNOWN"] =  new Screen(unknown, 0);
    
    ScreenText* readyLoaded = new ScreenText;
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE3));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE4));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE2));
    screenMap[HOME_STATE "_"] = new Screen(readyLoaded, READY_LOADED_LED_SEQ);
    
    ScreenText* startLoaded = new ScreenText;
    startLoaded->Add(new ScreenLine(START_LOADED_LINE1));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE3));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE4));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE5));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN1_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN2_LINE1));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN2_LINE2));
    screenMap[HOME_STATE "_Not yet defined"] = new Screen(startLoaded, START_LOADED_LED_SEQ);
    
    ScreenText* loadFail = new ScreenText;
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE1));
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE2));
    loadFail->Add(new ScreenLine(LOAD_FAIL_BTN1_LINE2));
    loadFail->Add(new ScreenLine(LOAD_FAIL_BTN2_LINE2));
    screenMap[HOME_STATE "_Download failed"] = new Screen(loadFail, LOAD_FAIL_LED_SEQ);
    
    ScreenText* printing = new ScreenText;
    printing->Add(new ScreenLine(PRINTING_LINE1));
    printing->Add(new ScreenLine(PRINTING_LINE2));
    printing->Add(new ScreenLine(PRINTING_LINE3));
    printing->Add(new ScreenLine(PRINTING_BTN1_LINE2));
    printing->Add(new ScreenLine(PRINTING_BTN2_LINE2));
    screenMap[PRINTING_STATE "_" ] = new Screen(printing, PRINTING_LED_SEQ);  
    screenMap[EXPOSING_STATE "_" ] = new Screen(printing, PRINTING_LED_SEQ);  
    screenMap[SEPARATING_STATE "_" ] = new Screen(printing, PRINTING_LED_SEQ);  
    
    ScreenText* paused = new ScreenText;
    paused->Add(new ScreenLine(PAUSED_LINE1));
    paused->Add(new ScreenLine(PAUSED_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN2_LINE1));
    paused->Add(new ScreenLine(PAUSED_BTN2_LINE2));
    screenMap[PAUSED_STATE "_"] = new Screen(paused, PAUSED_LED_SEQ);
    
}

