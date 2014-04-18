/* 
 * File:   PrinterStateMachine.cpp
 * Author: Richard Greene
 *
 * Implements all classes used by the PrintEngine's state machine.
 * 
 * Created on April 14, 2014, 10:55 AM
 */

#include <stdio.h>

#include <PrinterStateMachine.h>
#include <PrintEngine.h>
#include <Hardware.h>

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_pendingMotorEvent(None)
{
    printf("turning on printer\n");
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
    printf("turning off printer\n");
}

/// Either start a print or cancel the operation in progress
void PrinterStateMachine::StartOrCancelPrint()
{
    // if we're either in the Home or Idle states then request a print start
    if(state_cast<const Idle*>() != 0  || state_cast<const Home*>() != 0 )
        process_event(EvStartPrint());
    else    // cancel
        process_event(EvCancel());
    
}
   
/// Either pause or resume the print in progress
void PrinterStateMachine::PauseOrResume()
{
    // if we're in a printing state, pause
    if(state_cast<const Printing*>() != 0 )
        process_event(EvPause());
    else    // resume
        process_event(EvResume());    
}
   
/// Either put the printer to sleep or wake it up
void PrinterStateMachine::SleepOrWake()
{
     // if we're in an active state, sleep
    if(state_cast<const Active*>() != 0 )
        process_event(EvSleep());
    else    // wake
        process_event(EvWake());    
}

/// Sends the given command to the motor, and sets the given motor event as
/// the one that's pending.  Also sets the motor timeout.
void PrinterStateMachine::SetMotorCommand(const char command, 
                                      PendingMotorEvent pending)
{
    // send the command to the motor board
    PRINTENGINE->SendMotorCommand(command);
    // record the event to generate when that command is completed
    _pendingMotorEvent = pending; 
    // set the timeout (in future, may depend on the particular command))
    PRINTENGINE->StartMotorTimeoutTimer(DEFAULT_MOTOR_TIMEOUT_SEC);
}

/// Handle completion (or failure) of motor command)
void PrinterStateMachine::MotionCompleted(bool successfully)
{
    // disable the pending timeout
    PRINTENGINE->ClearMotorTimeoutTimer();

#ifdef DEBUG
    std::cout << "Motion completed  " << 
                (successfully ? "" : "un") <<
                "successfully with pending event " <<
                _pendingMotorEvent << std::endl;
#endif
    
    if(successfully)
    {
        switch(_pendingMotorEvent)
        {
            case None:
                perror(UNEXPECTED_MOTION_END);
                break;
                
            case AtHome:
                process_event(EvAtHome());
                break;
                
            case AtStartPosition:
                process_event(EvAtStartPosition());
                break;
                
            case Separated:
                process_event(EvSeparated());
                break;
                
            case AtLayer:
                process_event(EvAtLayer());
                break;
                
            default:
                perror(FormatError(UNKNOWN_MOTOR_EVENT, _pendingMotorEvent));
                break;
        }
    } 
    else
    {
        // we've already handled this error, 
        // just need to clear the pending event below
    }
    _pendingMotorEvent = None;
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
    
    // send the Home command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(HOME_COMMAND, AtHome);
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
    
    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
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
    
    // the timeout timer should already have been cleared, but this won't hurt
    PRINTENGINE->ClearMotorTimeoutTimer();
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

