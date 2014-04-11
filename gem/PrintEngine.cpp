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

sc::result Active::react(const EvDoorOpened&)
{
    // TODO: will need to save history
    return transit<DoorOpen>();
}

sc::result Active::react(const EvCancel&)
{
    return transit<Homing>();
}

sc::result Active::react(const EvError&)
{
    return transit<Idle>();
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
    return transit<Homing>();
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

DoorOpen::DoorOpen()
{
    printf("going to door open\n");
}

DoorOpen::~DoorOpen()
{
    printf("leaving 'door open' state\n");
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    // TODO: will need to invoke history
    return transit<Active>();
}

Homing::Homing()
{
    printf("homing\n");
}

Homing::~Homing()
{
    printf("stopping homing\n");
}

sc::result Homing::react(const EvAtHome&)
{
    return transit<Home>();
}

Idle::Idle()
{
    printf("idling\n");
}

Idle::~Idle()
{
    printf("leaving idle\n");
}

sc::result Idle::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperatire, etc.
    // also need to arrange to go straight to starting print after reaching home
    return transit<Homing>();
}

Home::Home()
{
    printf("at home\n");
}

Home::~Home()
{
    printf("leaving home\n");
}

sc::result Home::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, et.
    return transit<MovingToStartPosition>();
}

MovingToStartPosition::MovingToStartPosition()
{
    printf("moving to start position\n");
}

MovingToStartPosition::~MovingToStartPosition()
{
    printf("no longer moving to start position\n");
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperatire, etc.
    return transit<Exposing>();
}

Printing::Printing()
{
    printf("printing\n");
}

Printing::~Printing()
{
    printf("no longer printing\n");
}

sc::result Printing::react(const EvPause&)
{
    // TODO: need to save history
    return transit<Paused>();
}

Paused::Paused()
{
    printf("paused\n");
}

Paused::~Paused()
{
    printf("no longer paused\n");
}

sc::result Paused::react(const EvResume&)
{
    // TODO: need to restore from history
    return transit<Printing>();
}

Exposing::Exposing()
{
    printf("exposing\n");
}

Exposing::~Exposing()
{
    printf("no longer exposing\n");
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

Separating::Separating()
{
    printf("separating\n");
}

Separating::~Separating()
{
    printf("no longer separating\n");
}

sc::result Separating::react(const EvSeparated&)
{
    // if(no more layers)
    //     return transit<Homing>();
    // else
   return transit<MovingToLayer>();
}

MovingToLayer::MovingToLayer()
{
    printf("moving to layer\n");
}

MovingToLayer::~MovingToLayer()
{
    printf("no longer moving to layer\n");
}

sc::result MovingToLayer::react(const EvAtLayer&)
{
   return transit<Exposing>();
}





