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

#define PRINTENGINE context<PrinterStateMachine>()._pPrintEngine

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine)
{
    printf("turning on printer\n");
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
    printf("turning off printer\n");
}

PrinterOn::PrinterOn(my_context ctx) : my_base(ctx)
{
}

PrinterOn::~PrinterOn()
{

}

sc::result PrinterOn::react(const EvReset&)
{
    return transit<Initializing>();
}

Active::Active(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Active"); 
}

Active::~Active()
{
     PRINTENGINE->SendStatus("leaving Active");
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
    PRINTENGINE->SendStatus("entering Initializing");
    
    PRINTENGINE->Initialize();
    
    post_event(boost::intrusive_ptr<EvInitialized>( new EvInitialized() ));
}

Initializing::~Initializing()
{
    PRINTENGINE->SendStatus("leaving Initializing"); 
}

sc::result Initializing::react(const EvInitialized&)
{
    return transit<Homing>();
}

Sleeping::Sleeping(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Sleep"); 
}

Sleeping::~Sleeping()
{
    PRINTENGINE->SendStatus("leaving Sleep"); 
}

sc::result Sleeping::react(const EvWake&)
{
    return transit<sc::deep_history<Initializing> >();
}

DoorOpen::DoorOpen(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering DoorOpen"); 
}

DoorOpen::~DoorOpen()
{
    PRINTENGINE->SendStatus("leaving DoorOpen"); 
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    return transit<sc::deep_history<Initializing> >();
}

Homing::Homing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Homing"); 
}

Homing::~Homing()
{
    PRINTENGINE->SendStatus("leaving Homing"); 
}

sc::result Homing::react(const EvAtHome&)
{
    return transit<Home>();
}

Idle::Idle(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Idle"); 
}

Idle::~Idle()
{
    PRINTENGINE->SendStatus("leaving Idle"); 
}

sc::result Idle::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperatire, etc.
    // also need to arrange to go straight to starting print after reaching home
    return transit<Homing>();
}

Home::Home(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Home"); 
}

Home::~Home()
{
    PRINTENGINE->SendStatus("leaving Home"); 
}

sc::result Home::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, etc.
    return transit<MovingToStartPosition>();
}

MovingToStartPosition::MovingToStartPosition(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering MovingToStartPosition"); 
}

MovingToStartPosition::~MovingToStartPosition()
{
    PRINTENGINE->SendStatus("leaving MovingToStartPosition");
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, etc.
    return transit<Exposing>();
}

Printing::Printing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Printing");
    PRINTENGINE->EnablePulseTimer(true);
}

Printing::~Printing()
{
    PRINTENGINE->SendStatus("leaving Printing");
    PRINTENGINE->EnablePulseTimer(false);
}

sc::result Printing::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result Printing::react(const EvPulse&)
{
    // TODO: actually send updated status here
    PRINTENGINE->SendStatus("got pulse");
    return discard_event();
}

Paused::Paused(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Paused");
}

Paused::~Paused()
{
    PRINTENGINE->SendStatus("leaving Paused");
}

sc::result Paused::react(const EvResume&)
{  
    return transit<sc::deep_history<Printing> >();
}

Exposing::Exposing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Exposing");
    
    int layer = PRINTENGINE->NextLayer();
    
    // TODO show that layer
    
    PRINTENGINE->StartExposureTimer();
}

Exposing::~Exposing()
{
    PRINTENGINE->SendStatus("leaving Exposing");
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

Separating::Separating(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering Separating");
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus("leaving Separating");
}

sc::result Separating::react(const EvSeparated&)
{
    if(PRINTENGINE->NoMoreLayers())
        return transit<Homing>();
    else
        return transit<MovingToLayer>();
}

MovingToLayer::MovingToLayer(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("entering MovingToLayer");
}

MovingToLayer::~MovingToLayer()
{
    PRINTENGINE->SendStatus("leaving MovingToLayer");
}

sc::result MovingToLayer::react(const EvAtLayer&)
{
   return transit<Exposing>();
}

