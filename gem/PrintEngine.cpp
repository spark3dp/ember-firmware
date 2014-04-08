/* 
 * File:   PrintEngine.cpp
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#include <PrintEngine.h>
#include <stdio.h>


Printer::Printer()
{
    printf("turning on printer\n");
}

Printer::~Printer()
{
    printf("turning off printer\n");
}

sc::result Printer::react(const EvReset&)
{
    // TODO: apparently the state machine as a whole can't do a transit 
    // so how would we have it respond to reset? perh all 3 top level states 
    // need to handle it instead (or we need another all-encompassing state, 
    // PrinterOn, besides Printer state machine?)
//    return transit<Initializing>();
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




