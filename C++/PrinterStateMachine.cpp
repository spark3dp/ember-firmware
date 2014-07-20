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
#include <Logger.h>
#include <PrintData.h>

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_pendingMotorEvent(None),
_isProcessing(false)
{
    printf("turning on printer\n");
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
    printf("turning off printer\n");
}

/// Either start a print or pause or resume the print in progress
void PrinterStateMachine::StartPauseOrResume()
{
    // if we're either in the Home or Idle states then request a print start
    if(state_cast<const Idle*>() != 0  || state_cast<const Home*>() != 0 )
    {     
        _pPrintEngine->ClearError();
        _pPrintEngine->SetNumLayers(PrintData::GetNumLayers());       
        
        process_event(EvStartPrint());
    }
    else    // pause or resume
    {
        // if we're in a printing state, pause
        if(state_cast<const Printing*>() != 0 )
            process_event(EvPause());
        else if(state_cast<const Paused*>() != 0 )   // resume
            process_event(EvResume());  
    }    
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
                PRINTENGINE->HandleError(UNEXPECTED_MOTION_END);
                break;
                
            case Initialized:
                _pendingMotorEvent = None;
                process_event(EvInitialized()); 
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
                PRINTENGINE->HandleError(UNKNOWN_MOTOR_EVENT, false, NULL, 
                                         _pendingMotorEvent);
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

/// Overrides base type behavior by flagging when we are currently processing.
void PrinterStateMachine::process_event( const event_base_type & evt )
{
    _isProcessing = true;
    sc::state_machine< PrinterStateMachine, PrinterOn >::process_event(evt);
    _isProcessing = false;    
}

/// Handle an error that prevents printing or moving the motors, by going to the 
/// Idle state.
void PrinterStateMachine::HandleFatalError()
{
    // we can only call process_event if we aren't already processing an event
    if(_isProcessing)
        post_event(EvError());
    else
        process_event(EvError());
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

DoorClosed::DoorClosed(my_context ctx) : my_base(ctx),
_startRequestedFromIdle(false)
{
    PRINTENGINE->SendStatus("Door Closed", Entering); 
}

DoorClosed::~DoorClosed()
{
     PRINTENGINE->SendStatus("Door Closed", Leaving);
}

sc::result DoorClosed::react(const EvDoorOpened&)
{
    return transit<DoorOpen>();
}

sc::result DoorClosed::react(const EvCancel&)
{
    PRINTENGINE->CancelPrint();
    context<DoorClosed>().StartRequestedFromIdle(false);    
    return transit<Homing>();
}

sc::result DoorClosed::react(const EvError&)
{
    PRINTENGINE->CancelPrint();
    return transit<Idle>();
}

Initializing::Initializing(my_context ctx) : my_base(ctx)
{    
    PRINTENGINE->SendStatus("Initializing", Entering);
    
    PRINTENGINE->Initialize();
    context<DoorClosed>().StartRequestedFromIdle(false);
    // send the initialization command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(ACK, Initialized);                                     
}

Initializing::~Initializing()
{
    PRINTENGINE->SendStatus("Initializing", Leaving); 
}

sc::result Initializing::react(const EvInitialized&)
{
    return transit<Homing>();
}

DoorOpen::DoorOpen(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Door Open", Entering); 
}

DoorOpen::~DoorOpen()
{
    PRINTENGINE->SendStatus("Door Open", Leaving); 
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    return transit<sc::deep_history<Initializing> >();
}

Homing::Homing(my_context ctx) : my_base(ctx)
{            
    PRINTENGINE->SendStatus("Homing", Entering); 
    
    // check to see if the door is open on startup
    if(PRINTENGINE->DoorIsOpen())
    {
        post_event(boost::intrusive_ptr<EvDoorOpened>( new EvDoorOpened() ));
    }
    else
    {
        // send the Home command to the motor board, and
        // record the motor board event we're waiting for
        context<PrinterStateMachine>().SetMotorCommand(HOME_COMMAND, AtHome, 
                                                      LONGER_MOTOR_TIMEOUT_SEC);
    }
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
    
    context<DoorClosed>().StartRequestedFromIdle(false);
    
    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Idle::~Idle()
{
    PRINTENGINE->ClearError();
    PRINTENGINE->SendStatus("Idle", Leaving); 
}

sc::result Idle::react(const EvStartPrint&)
{
    // go straight to attempting to start the print after reaching home
    // the actual start will be qualified there by presence of valid data etc.
    context<DoorClosed>().StartRequestedFromIdle(true);
    
    return transit<Homing>();
}

Home::Home(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Home", Entering); 
    
    // the timeout timer should already have been cleared, but this won't hurt
    PRINTENGINE->ClearMotorTimeoutTimer();
    
    if(context<DoorClosed>().StartRequestedFromIdle())
    {
        // we got here due to a start requested from the Idle state
        // so attempt to start the print now
        post_event(boost::intrusive_ptr<EvStartPrint>( new EvStartPrint() ));
    }
    context<DoorClosed>().StartRequestedFromIdle(false);    
}

Home::~Home()
{
    PRINTENGINE->SendStatus("Home", Leaving); 
}

sc::result Home::react(const EvStartPrint&)
{
    if(PRINTENGINE->TryStartPrint())
        return transit<PrintSetup>();
    else
        return discard_event(); // error will have already been reported
}

PrintSetup::PrintSetup(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("PrintSetup", Entering);
    PRINTENGINE->SendSettings();
}

sc::result PrintSetup::react(const EvGotSetting&)
{
    if(PRINTENGINE->SendSettings())
        return transit<MovingToStartPosition>();
    else
        return discard_event(); // further setup is still needed
}

PrintSetup::~PrintSetup()
{
    PRINTENGINE->SendStatus("PrintSetup", Leaving);
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
    PRINTENGINE->DecreaseEstimatedPrintTime(PULSE_PERIOD_SEC);
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
    return transit<sc::deep_history<EndingPrint> >();
}

int Exposing::_remainingExposureTimeSec = 0;
int Exposing::_previousLayer = 0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus("Exposing", Entering);
    
    int exposureTimeSec;
    if(_remainingExposureTimeSec > 0)
    {
        // we must be returning here after door open, pause, or sleep
        exposureTimeSec = _remainingExposureTimeSec;
        int layer = _previousLayer;
        PRINTENGINE->SetCurrentLayer(layer);
        
        PRINTENGINE->SetEstimatedPrintTime(true);
        // adjust the estimated remaining print time by the remaining exposure time
        PRINTENGINE->DecreaseEstimatedPrintTime(
                PRINTENGINE->GetExposureTimeSec() - _remainingExposureTimeSec);  
    }
    else
    { 
        PRINTENGINE->NextLayer();
        exposureTimeSec = PRINTENGINE->GetExposureTimeSec();
        PRINTENGINE->SetEstimatedPrintTime(true);
    }
    
    // display current layer
    PRINTENGINE->ShowImage();
    
    PRINTENGINE->StartExposureTimer(exposureTimeSec);
}

Exposing::~Exposing()
{
    // black out the projected image
    PRINTENGINE->ShowBlack();
    
    // if we're leaving during the middle of exposure, 
    // we need to record that fact, 
    // as well as our layer and the remaining exposure time
    _remainingExposureTimeSec = PRINTENGINE->GetRemainingExposureTimeSec();
    if(_remainingExposureTimeSec > 0)
    {
        _previousLayer = PRINTENGINE->GetCurrentLayer();
    }
    PRINTENGINE->SendStatus("Exposing", Leaving);
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

/// Clear the information saved when leaving Exposing before the exposure is 
/// actually completed
void Exposing::ClearPendingExposureInfo()
{
    _remainingExposureTimeSec = 0;
    _previousLayer = 0;    
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

