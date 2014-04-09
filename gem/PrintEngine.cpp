/* 
 * File:   PrintEngine.cpp
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#include <PrintEngine.h>
#include <stdio.h>


PrinterStateMachine::PrinterStateMachine()
{
    printf("turning on printer\n");
}

PrinterStateMachine::~PrinterStateMachine()
{
    printf("turning off printer\n");
}

PrinterOn::PrinterOn()
{
    printf("going to 'printer on' super state\n");
}

PrinterOn::~PrinterOn()
{
    printf("leaving 'printer on' super state\n");
}

sc::result PrinterOn::react(const EvReset&)
{
    return transit<Initializing>();
}

Active::Active()
{
    printf("going to active\n");
}

Active::~Active()
{
    printf("leaving active\n");
}

sc::result Active::react(const EvSleep&)
{
    // TODO: will need to save history
    return transit<Sleeping>();
}

Initializing::Initializing()
{
    printf("initializing\n");
}

Initializing::~Initializing()
{
}

sc::result Initializing::react(const EvInitialized&)
{
 //   return transit<Homing>();
}

Sleeping::Sleeping()
{
    printf("going to sleep\n");
}

Sleeping::~Sleeping()
{
    printf("leaving sleep\n");
}

sc::result Sleeping::react(const EvWake&)
{
    // TODO: will need to invoke history
    return transit<Active>();
}




