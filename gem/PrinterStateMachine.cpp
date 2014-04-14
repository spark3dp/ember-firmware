/* 
 * File:   PrinterStateMachine.cpp
 * Author: Richard Greene
 *
 * Implements all classes used by the PrintEngine's state machine.
 * 
 * Created on April 14, 2014, 10:55 AM
 */

#include <PrinterStateMachine.h>
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

Initializing::Initializing(my_context ctx) : my_base(ctx)
{
    PrintEngine::Instance().SendStatus("entering Initializing");
    
    PrintEngine::Instance().Initialize();
    
    post_event(boost::intrusive_ptr<EvInitialized>( new EvInitialized() ));
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
    // low-enough temperature, etc.
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
    // low-enough temperature, etc.
    return transit<Exposing>();
}

Printing::Printing()
{
    PrintEngine::Instance().SendStatus("entering Printing");
    PrintEngine::Instance().EnablePulseTimer(true);
}

Printing::~Printing()
{
    PrintEngine::Instance().SendStatus("leaving Printing");
    PrintEngine::Instance().EnablePulseTimer(false);
}

sc::result Printing::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result Printing::react(const EvPulse&)
{
    // TODO: actually send updated status here
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
    
    int layer = PrintEngine::Instance().NextLayer();
    
    // TODO show that layer
    
    PrintEngine::Instance().StartExposureTimer();
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
    if(PrintEngine::Instance().NoMoreLayers())
        return transit<Homing>();
    else
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

