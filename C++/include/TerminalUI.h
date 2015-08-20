/* 
 * File:   TerminalUI.h
 * Author: Richard Greene
 * 
 * Defines the terminal as a UI component, for getting status updates.
 * (Terminal input is also accepted, via stdin in EventHandler.)
 *
 * Created on April 18, 2014, 4:54 PM
 */

#ifndef TERMINALUI_H
#define	TERMINALUI_H

#include "ICallback.h"

class TerminalUI : public ICallback
{ 
public:    
    
private:
    void Callback(EventType eventType, const EventData& data);
};

#endif	/* TERMINALUI_H */

