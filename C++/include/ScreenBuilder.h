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

class ScreenBuilder {
public:
    static void BuildScreens(std::map<std::string, Screen*>& screenMap);
    
private:

};

#endif	/* SCREENBUILDER_H */

