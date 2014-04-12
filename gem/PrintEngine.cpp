/* 
 * File:   PrintEngine.cpp
 * Author: Richard Greene
 *
 * Created on April 8, 2014, 2:18 PM
 */

#include <PrintEngine.h>
#include <stdio.h>

void PrintEngine::SendStatus(const char* stateName)
{
    _status._state = stateName;
    // TODO arrange to update and send actual status
    // for now, just print out what state we're in
    printf("%s\n", _status._state);
}


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
 }

PrinterOn::~PrinterOn()
{

}

sc::result PrinterOn::react(const EvReset&)
{
    return transit<Initializing>();
}

Active::Active()
{
    PrintEngine::Instance().SendStatus("entering Active"); 
}

Active::~Active()
{
     PrintEngine::Instance().SendStatus("leaving Active");
}

sc::result Active::react(const EvSleep&)
{
    return transit<Sleeping>();
}

sc::result Active::react(const EvDoorOpened&)
{
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
    PrintEngine::Instance().SendStatus("entering Initializing"); 
}

Initializing::~Initializing()
{
    PrintEngine::Instance().SendStatus("leaving Initializing"); 
}

sc::result Initializing::react(const EvInitialized&)
{
    return transit<Homing>();
}

Sleeping::Sleeping()
{
    PrintEngine::Instance().SendStatus("entering Sleep"); 
}

Sleeping::~Sleeping()
{
    PrintEngine::Instance().SendStatus("leaving Sleep"); 
}

sc::result Sleeping::react(const EvWake&)
{
    return transit<sc::deep_history<Initializing> >();
}

DoorOpen::DoorOpen()
{
    PrintEngine::Instance().SendStatus("entering DoorOpen"); 
}

DoorOpen::~DoorOpen()
{
    PrintEngine::Instance().SendStatus("leaving DoorOpen"); 
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    return transit<sc::deep_history<Initializing> >();
}

Homing::Homing()
{
    PrintEngine::Instance().SendStatus("entering Homing"); 
}

Homing::~Homing()
{
    PrintEngine::Instance().SendStatus("leaving Homing"); 
}

sc::result Homing::react(const EvAtHome&)
{
    return transit<Home>();
}

Idle::Idle()
{
    PrintEngine::Instance().SendStatus("entering Idle"); 
}

Idle::~Idle()
{
    PrintEngine::Instance().SendStatus("leaving Idle"); 
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
    PrintEngine::Instance().SendStatus("entering Home"); 
}

Home::~Home()
{
    PrintEngine::Instance().SendStatus("leaving Home"); 
}

sc::result Home::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, et.
    return transit<MovingToStartPosition>();
}

MovingToStartPosition::MovingToStartPosition()
{
    PrintEngine::Instance().SendStatus("entering MovingToStartPosition"); 
}

MovingToStartPosition::~MovingToStartPosition()
{
    PrintEngine::Instance().SendStatus("leaving MovingToStartPosition");
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperatire, etc.
    return transit<Exposing>();
}

Printing::Printing()
{
    PrintEngine::Instance().SendStatus("entering Printing");
}

Printing::~Printing()
{
    PrintEngine::Instance().SendStatus("leaving Printing");
}

sc::result Printing::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result Printing::react(const EvPulse&)
{
    // TODO: actually update status here
    PrintEngine::Instance().SendStatus("got pulse");
    return discard_event();
}

Paused::Paused()
{
    PrintEngine::Instance().SendStatus("entering Paused");
}

Paused::~Paused()
{
    PrintEngine::Instance().SendStatus("leaving Paused");
}

sc::result Paused::react(const EvResume&)
{  
    return transit<sc::deep_history<Printing> >();
}

Exposing::Exposing()
{
    PrintEngine::Instance().SendStatus("entering Exposing");
}

Exposing::~Exposing()
{
    PrintEngine::Instance().SendStatus("leaving Exposing");
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

Separating::Separating()
{
    PrintEngine::Instance().SendStatus("entering Separating");
}

Separating::~Separating()
{
    PrintEngine::Instance().SendStatus("leaving Separating");
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
    PrintEngine::Instance().SendStatus("entering MovingToLayer");
}

MovingToLayer::~MovingToLayer()
{
    PrintEngine::Instance().SendStatus("leaving MovingToLayer");
}

sc::result MovingToLayer::react(const EvAtLayer&)
{
   return transit<Exposing>();
}





