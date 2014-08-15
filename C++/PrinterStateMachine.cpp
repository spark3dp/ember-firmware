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
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
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
                PRINTENGINE->HandleError(UnexpectedMotionEnd);
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
                PRINTENGINE->HandleError(UnknownMotorEvent, false, NULL, 
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

/// Overrides (hides) base type behavior by flagging when we are in the middle
/// of processing.
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

/// Returns true if and only if we've started and not yet completed a print 
bool PrinterStateMachine::IsPrinting()
{
    return (state_cast<const Printing*>() != 0  || 
            state_cast<const PrintSetup*>() != 0 ||
            state_cast<const MovingToStartPosition*>() != 0);
}

PrinterOn::PrinterOn(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PrinterOnState, Entering);
}
    
PrinterOn::~PrinterOn()
{
    PRINTENGINE->SendStatus(PrinterOnState, Leaving);
}

sc::result PrinterOn::react(const EvReset&)
{
    PRINTENGINE->CancelPrint();
    return transit<Initializing>();
}

sc::result PrinterOn::react(const EvShowVersion&)
{
    return transit<ShowingVersion>();
}

ShowingVersion::ShowingVersion(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(ShowingVersionState, Entering);
}

ShowingVersion::~ShowingVersion()
{
    PRINTENGINE->SendStatus(ShowingVersionState, Leaving);
}

sc::result ShowingVersion::react(const EvLeftButton&)
{
    // leave the version screen, returning whence we came
    return transit<sc::deep_history<Idle> >();
}

DoorClosed::DoorClosed(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(DoorClosedState, Entering); 
}

DoorClosed::~DoorClosed()
{
    PRINTENGINE->SendStatus(DoorClosedState, Leaving);
}

sc::result DoorClosed::react(const EvDoorOpened&)
{
    return transit<DoorOpen>();
}

sc::result DoorClosed::react(const EvRequestCancel&)
{
    return transit<ConfirmCancel>();
}

sc::result DoorClosed::react(const EvError&)
{
    PRINTENGINE->CancelPrint();
    return transit<Idle>();
}

Initializing::Initializing(my_context ctx) : my_base(ctx)
{    
    PRINTENGINE->SendStatus(InitializingState, Entering);
    
    PRINTENGINE->Initialize();
    // send the initialization command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(ACK, Initialized);                                     
}

Initializing::~Initializing()
{
    PRINTENGINE->SendStatus(InitializingState, Leaving); 
}

sc::result Initializing::react(const EvInitialized&)
{
    return transit<Homing>();
}

DoorOpen::DoorOpen(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(DoorOpenState, Entering); 
    
    PRINTENGINE->PowerProjector(false);
}

DoorOpen::~DoorOpen()
{
    PRINTENGINE->SendStatus(DoorOpenState, Leaving); 
    
    PRINTENGINE->PowerProjector(true);
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    // arrange to clear the screen first
    PRINTENGINE->SendStatus(DoorOpenState, NoChange, ExitingDoorOpen); 
    
    return transit<sc::deep_history<Initializing> >();
}

Homing::Homing(my_context ctx) : my_base(ctx)
{            
    PRINTENGINE->SendStatus(HomingState, Entering); 
    
    // check to see if the door is open on startup
    if(PRINTENGINE->DoorIsOpen())
    {
        post_event(EvDoorOpened());
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
    PRINTENGINE->SendStatus(HomingState, Leaving); 
}

sc::result Homing::react(const EvAtHome&)
{
    return transit<Home>();
}

Idle::Idle(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(IdleState, Entering);  
    PRINTENGINE->PowerProjector(false);  
    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Idle::~Idle()
{
    PRINTENGINE->SendStatus(IdleState, Leaving); 
}

sc::result Idle::react(const EvLeftButton&)
{   
    PRINTENGINE->ClearError();
    PRINTENGINE->PowerProjector(true);
    PRINTENGINE->SendStatus(IdleState, NoChange, ClearingError); 
    return transit<Homing>();
}

sc::result Idle::react(const EvRightButton&)
{   
    post_event(EvReset());
    return discard_event();
}

sc::result Idle::react(const EvRightButtonHold&)
{
    post_event(EvShowVersion());
    return discard_event();    
}

ConfirmCancel::ConfirmCancel(my_context ctx): my_base(ctx)
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Entering);  
}

ConfirmCancel::~ConfirmCancel()
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Leaving);      
}

sc::result ConfirmCancel::react(const EvCancel&)    
{    
    PRINTENGINE->CancelPrint();
    return transit<Homing>();
}

sc::result ConfirmCancel::react(const EvLeftButton&)    
{    
    post_event(EvCancel());
    return discard_event();
}

sc::result ConfirmCancel::react(const EvNoCancel&)    
{    
    transit<sc::deep_history<Initializing> >();
}

sc::result ConfirmCancel::react(const EvRightButton&)    
{    
    post_event(EvNoCancel());
    return discard_event();   
}


Home::Home(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(HomeState, Entering, 
                 PRINTENGINE->HasPrintData() ? HavePrintData : NoPrintData); 
    
    // the timeout timer should already have been cleared, but this won't hurt
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Home::~Home()
{
    PRINTENGINE->SendStatus(HomeState, Leaving); 
}

sc::result Home::TryStartPrint()
{
    if(PRINTENGINE->TryStartPrint())
        return transit<PrintSetup>();
    else
        return discard_event(); // error will have already been reported
}

sc::result Home::react(const EvStartPrint&)
{
    return TryStartPrint();
}

sc::result Home::react(const EvLeftButton&)
{
    switch(PRINTENGINE->GetUISubState())
    {
        case NoPrintData:
        case Downloading:
            // ignore button press when nothing to print or download in progress
            return discard_event(); 
            break;
            
        case DownloadFailed:
            // just refresh the home screen with the appropriate message
            PRINTENGINE->SendStatus(HomeState, NoChange, 
                 PRINTENGINE->HasPrintData() ? HavePrintData : NoPrintData); 
            return discard_event(); 
            break;
            
        default:
            return TryStartPrint();
            break;
    }
}

sc::result Home::react(const EvRightButtonHold&)
{
    post_event(EvShowVersion());
    return discard_event();    
}

PrintSetup::PrintSetup(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PrintSetupState, Entering);
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
    PRINTENGINE->SendStatus(PrintSetupState, Leaving);
}

MovingToStartPosition::MovingToStartPosition(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Entering); 
    // send the move to layer command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(MOVE_TO_START_POSN_COMMAND, 
                                   AtStartPosition, LONGEST_MOTOR_TIMEOUT_SEC);
}

MovingToStartPosition::~MovingToStartPosition()
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Leaving);
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    return transit<Exposing>();
}

sc::result MovingToStartPosition::react(const EvRightButton&)
{
    post_event(EvRequestCancel());
    return discard_event();         
}
 
Printing::Printing(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PrintingState, Entering);
}

Printing::~Printing()
{
    PRINTENGINE->SendStatus(PrintingState, Leaving);
}

sc::result Printing::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result Printing::react(const EvLeftButton&)
{
    post_event(EvPause());
    return discard_event();         
}

sc::result Printing::react(const EvRightButton&)
{
    post_event(EvRequestCancel());
    return discard_event();         
}

Paused::Paused(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PausedState, Entering);
}

Paused::~Paused()
{
    PRINTENGINE->SendStatus(PausedState, Leaving);
}

sc::result Paused::react(const EvResume&)
{  
    return transit<sc::deep_history<EndingPrint> >();
}

sc::result Paused::react(const EvLeftButton&)
{
    post_event(EvResume());
    return discard_event();         
}

sc::result Paused::react(const EvRightButton&)
{
    post_event(EvRequestCancel());
    return discard_event();         
}

double Exposing::_remainingExposureTimeSec = 0.0;
int Exposing::_previousLayer = 0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{
    // calculate time estimate before sending status
    double exposureTimeSec;
    if(_remainingExposureTimeSec > 0)
    {
        // we must be returning here after door open or pause
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
       
    // this update will convey the remaining print time to UI components
    PRINTENGINE->SendStatus(ExposingState, Entering);

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
    PRINTENGINE->SendStatus(ExposingState, Leaving);
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
    PRINTENGINE->SendStatus(SeparatingState, Entering);
    
    // send the separating command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(SEPARATE_COMMAND, Separated);
    
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus(SeparatingState, Leaving);
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
    PRINTENGINE->SendStatus(EndingPrintState, Entering);
    
    // send the print ending command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(END_PRINT_COMMAND, PrintEnded);
}

EndingPrint::~EndingPrint()
{
    PRINTENGINE->CancelPrint();    
    PRINTENGINE->SendStatus(EndingPrintState, Leaving);
}

sc::result EndingPrint::react(const EvPrintEnded&)
{
    return transit<Homing>();
}

