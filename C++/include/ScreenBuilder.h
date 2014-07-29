/* 
 * File:   ScreenBuilder.h
 * Author: Richard Greene
 * 
 * Builds the screens shown on the front panel.
 * 
 * Created on July 23, 2014, 12:15 PM
 */

#ifndef SCREENBUILDER_H
#define	SCREENBUILDER_H

#include<map>

#include <Screen.h>
#include <PrinterStatus.h>

typedef int ScreenKey;

#define UNKNOWN_SCREEN_KEY (-1)

class ScreenBuilder {
public:
    static void BuildScreens(std::map<int, Screen*>& screenMap);
    static ScreenKey GetKey(PrintEngineState state, UISubState subState);
};

#endif	/* SCREENBUILDER_H */

