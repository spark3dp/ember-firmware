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
                                      PendingMotorEvent pending,
                                      int timeoutSec)
{
    // send the command to the motor board
    PRINTENGINE->SendMotorCommand(command);
    // record the event to generate when the command is completed
    _pendingMotorEvent = pending;   
    // set the timeout (in future, may depend on the particular command))
    PRINTENGINE->StartMotorTimeoutTimer(timeoutSec);
}

/// Handle completion (or failure) of motor command)
/// Note: we need to clear the pending motor event just before processing new 
/// state machine events, in case those also want to send a new motor command 
void PrinterStateMachine::MotionCompleted(bool successfully)
{
    // disable the pending timeout
    PRINTENGINE->ClearMotorTimeoutTimer();

#ifdef DEBUG
//    std::cout << "Motion completed  " << 
//                (successfully ? "" : "un") <<
//                "successfully with pending event " <<
//                _pendingMotorEvent << std::endl;
#endif
    
    if(successfully)
    {
        switch(_pendingMotorEvent)
        {
            case None:
                perror(UNEXPECTED_MOTION_END);
                break;
                
            case AtHome:
                _pendingMotorEvent = None;
                process_event(EvAtHome());
                break;
                
            case AtStartPosition:
                _pendingMotorEvent = None;
                process_event(EvAtStartPosition());
                break;
                
            case Separated:
                _pendingMotorEvent = None;
                process_event(EvSeparated());
                break;
                
            case PrintEnded:
                _pendingMotorEvent = None;
                process_event(EvPrintEnded());
                break;
                
            default:
                perror(FormatError(UNKNOWN_MOTOR_EVENT, _pendingMotorEvent));
                _pendingMotorEvent = None;
                break;
        }
    } 
    else
    {
        // we've already handled this error, 
        // just need to clear the pending event 
        _pendingMotorEvent = None;
    }    
}

PrinterOn::PrinterOn(my_context ctx) : my_base(ctx)
{
}


    
PrinterOn::~PrinterOn()
{

}

sc::result PrinterOn::react(const EvReset&)
{
    PRINTENGINE->CancelPrint();
    return transit<Initializing>();
}

Active::Active(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Active", Entering); 
}

Active::~Active()
{
     PRINTENGINE->SendStatus("Active", Leaving);
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
    PRINTENGINE->CancelPrint();    
    return transit<Homing>();
}

sc::result Active::react(const EvError&)
{
    PRINTENGINE->CancelPrint();
    return transit<Idle>();
}

Initializing::Initializing(my_context ctx) : my_base(ctx)
{    
    PRINTENGINE->SendStatus("Initializing", Entering);
    
    PRINTENGINE->Initialize();
    
    post_event(boost::intrusive_ptr<EvInitialized>( new EvInitialized() ));
}

Initializing::~Initializing()
{
    PRINTENGINE->SendStatus("Initializing", Leaving); 
}

sc::result Initializing::react(const EvInitialized&)
{
    return transit<Homing>();
}

Sleeping::Sleeping(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Sleep", Entering); 
}

Sleeping::~Sleeping()
{
    PRINTENGINE->SendStatus("Sleep", Leaving); 
}

sc::result Sleeping::react(const EvWake&)
{
    return transit<sc::deep_history<Initializing> >();
}

DoorOpen::DoorOpen(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Door Open", Entering); 
}

DoorOpen::~DoorOpen()
{
    PRINTENGINE->SendStatus("DoorOpen", Leaving); 
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    return transit<sc::deep_history<Initializing> >();
}

Homing::Homing(my_context ctx) : my_base(ctx)
{    
    PRINTENGINE->SendStatus("Homing", Entering); 
    
    // send the Home command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(HOME_COMMAND, AtHome, 
                                                   LONGER_MOTOR_TIMEOUT_SEC);
}

Homing::~Homing()
{
    PRINTENGINE->SendStatus("Homing", Leaving); 
}

sc::result Homing::react(const EvAtHome&)
{
    return transit<Home>();
}

Idle::Idle(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Idle", Entering); 
    
    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Idle::~Idle()
{
    PRINTENGINE->SendStatus("Idle", Leaving); 
}

sc::result Idle::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, etc.
    // also need to arrange to go straight to starting print after reaching home
    return transit<Homing>();
}

Home::Home(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Home", Entering); 
    
    // the timeout timer should already have been cleared, but this won't hurt
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Home::~Home()
{
    PRINTENGINE->SendStatus("Home", Leaving); 
}

sc::result Home::react(const EvStartPrint&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, etc.
    return transit<MovingToStartPosition>();
}

MovingToStartPosition::MovingToStartPosition(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("MovingToStartPosition", Entering); 
    // send the move to layer command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(MOVE_TO_START_POSN_COMMAND, 
                                   AtStartPosition, LONGEST_MOTOR_TIMEOUT_SEC);
}

MovingToStartPosition::~MovingToStartPosition()
{
    PRINTENGINE->SendStatus("MovingToStartPosition", Leaving);
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    // TODO: need to qualify this by presence of valid data, 
    // low-enough temperature, etc.
    // and get number of layers etc. from settings
    PRINTENGINE->SetNumLayers(3);
    PRINTENGINE->SetEstimatedPrintTime(true);
    return transit<Exposing>();
}

Printing::Printing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Printing", Entering);
    PRINTENGINE->EnablePulseTimer(true);
}

Printing::~Printing()
{
    PRINTENGINE->SendStatus("Printing", Leaving);
    PRINTENGINE->EnablePulseTimer(false);
}

sc::result Printing::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result Printing::react(const EvPulse&)
{
    PRINTENGINE->UpdateRemainingPrintTime();
    PRINTENGINE->SendStatus("Printing");
    return discard_event();
}

Paused::Paused(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Paused", Entering);
}

Paused::~Paused()
{
    PRINTENGINE->SendStatus("Paused", Leaving);
}

sc::result Paused::react(const EvResume&)
{  
    return transit<sc::deep_history<Printing> >();
}

int Exposing::_remainingExposureTimeSec = 0;
int Exposing::_previousLayer = 0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Exposing", Entering);
    
    int exposureTimeSec;
    int layer;
    if(_remainingExposureTimeSec > 0)
    {
        // we must be returning here after door open, pause, or sleep
        exposureTimeSec = _remainingExposureTimeSec;
        layer = _previousLayer;
    }
    else
    {
        exposureTimeSec = PRINTENGINE->GetExposureTimeSec();
        layer = PRINTENGINE->NextLayer();
    }
    
    // TODO display 'layer'
    
    PRINTENGINE->StartExposureTimer(exposureTimeSec);
}

Exposing::~Exposing()
{
    // TODO: black out the projected image
    
    // if we're leaving during the middle of exposure, 
    // we need to record that fact, 
    // as well as our layer and the remaining exposure time
    _remainingExposureTimeSec = PRINTENGINE->GetRemainingExposureTimeSec();
    if(_remainingExposureTimeSec > 0)
    {
         std::cout << "remaining exp time " << _remainingExposureTimeSec << std::endl;
        _previousLayer = PRINTENGINE->CurrentLayer();
    }
    PRINTENGINE->SendStatus("Exposing", Leaving);
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

Separating::Separating(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Separating", Entering);
    
    // send the separating command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(SEPARATE_COMMAND, Separated);
    
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus("Separating", Leaving);
}

sc::result Separating::react(const EvSeparated&)
{
    if(PRINTENGINE->NoMoreLayers())
    {
        // clear the print-in-progress status
        PRINTENGINE->SetEstimatedPrintTime(false);
        return transit<EndingPrint>();
    }
    else
        return transit<Exposing>();
}

EndingPrint::EndingPrint(my_context ctx) : my_base(ctx)
{    
    PRINTENGINE->SendStatus("EndingPrint", Entering);
    
    // send the print ending command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(END_PRINT_COMMAND, PrintEnded);
}

EndingPrint::~EndingPrint()
{
    PRINTENGINE->CancelPrint();    
    PRINTENGINE->SendStatus("EndingPrint", Leaving);
}

sc::result EndingPrint::react(const EvPrintEnded&)
{
    return transit<Homing>();
}

