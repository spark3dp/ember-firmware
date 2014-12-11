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
#include <utils.h>

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_pendingMotorEvent(None),
_isProcessing(false),
_homingSubState(NoUISubState),
_pausedSubState(NoUISubState)       
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
/// Error state.
void PrinterStateMachine::HandleFatalError()
{
    // we can only call process_event if we aren't already processing an event
    if(_isProcessing)
        post_event(EvError());
    else
        process_event(EvError());
}

/// Determine if motor movement is in progress, i.e. if we're awaiting 
/// completion of a motor command.
bool PrinterStateMachine::IsMotorMoving()
{
    return _pendingMotorEvent != None;
}

/// Perform common actions needed when canceling a print in progress.
void PrinterStateMachine::CancelPrint()
{
    PRINTENGINE->ClearCurrentPrint();
    _homingSubState = PrintCanceled;
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
    PRINTENGINE->ClearCurrentPrint();
    return transit<Initializing>();
}

sc::result PrinterOn::react(const EvShowVersion&)
{
    return transit<ShowingVersion>();
}

sc::result PrinterOn::react(const EvError&)
{
    PRINTENGINE->ClearCurrentPrint();
    return transit<Error>();
}

ShowingVersion::ShowingVersion(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(ShowingVersionState, Entering);
}

ShowingVersion::~ShowingVersion()
{
    PRINTENGINE->SendStatus(ShowingVersionState, Leaving);
}

sc::result ShowingVersion::react(const EvRightButton&)
{
    // leave the version screen, returning whence we came
    return transit<sc::deep_history<Error> >();
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

DoorOpen::DoorOpen(my_context ctx) : 
my_base(ctx),
_atStartPosition(false),        
_separated(false)
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
    
    if(_atStartPosition)
    {
        // we got to the start position when the door was open, 
        // so we just need to start exposing now
        return transit<Exposing>();
    }
    else if(_separated)
    {
        // we completed a separation when the door was open, 
        // so we just need to end the print or start exposing now
        if(PRINTENGINE->NoMoreLayers())
            return transit<EndingPrint>();
        else
            return transit<Exposing>();   
    }
    else
        return transit<sc::deep_history<Initializing> >();
}

sc::result DoorOpen::react(const EvAtStartPosition&)
{
    // since we got to the start position when the door was open, 
    // we'll want to go to Exposing state when door is closed
    _atStartPosition = true;
}

sc::result DoorOpen::react(const EvSeparated&)
{
    // since we separated when the door was open, we'll want to go to the next
    // appropriate state when door is closed
     _separated = true;
}

Homing::Homing(my_context ctx) : my_base(ctx)
{            
    PRINTENGINE->SendStatus(HomingState, Entering, 
                            context<PrinterStateMachine>()._homingSubState); 
    
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
    context<PrinterStateMachine>()._homingSubState = NoUISubState;
    
    // previous job ID no longer applies
    PRINTENGINE->ClearJobID();
    
    return transit<Home>();
}

Error::Error(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(ErrorState, Entering);  
    PRINTENGINE->PowerProjector(false);  
    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Error::~Error()
{
    PRINTENGINE->PowerProjector(true);
    PRINTENGINE->SendStatus(ErrorState, Leaving); 
}

sc::result Error::react(const EvRightButton&)
{   
    PRINTENGINE->ClearError();
    return transit<Homing>();
}

sc::result Error::react(const EvLeftButton&)
{   
    post_event(EvReset());
    return discard_event();
}

sc::result Error::react(const EvLeftButtonHold&)
{
    post_event(EvShowVersion());
    return discard_event();    
}

Calibrate::Calibrate(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(CalibrateState, Entering);
}
   
Calibrate::~Calibrate()
{
    PRINTENGINE->SendStatus(CalibrateState, Leaving);         
}

sc::result Calibrate::react(const EvRightButton&)
{
    return transit<MovingToCalibration>();    
}
    
sc::result Calibrate::react(const EvLeftButton&)
{
    return transit<EndingCalibration>();    
}

MovingToCalibration::MovingToCalibration(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToCalibrationState, Entering); 

    // send the move to start position command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(MOVE_TO_START_POSN_COMMAND, 
                                   AtStartPosition, LONGEST_MOTOR_TIMEOUT_SEC);    
}

MovingToCalibration::~MovingToCalibration()
{
    PRINTENGINE->SendStatus(MovingToCalibrationState, Leaving); 
}

sc::result MovingToCalibration::react(const EvAtStartPosition&)
{
    return transit<Calibrating>(); 
}

Calibrating::Calibrating(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(CalibratingState, Entering);     
}
   
Calibrating::~Calibrating()
{
    PRINTENGINE->SendStatus(CalibratingState, Leaving);         
}

sc::result Calibrating::react(const EvRightButton&)
{
    return transit<EndingCalibration>();    
}   

EndingCalibration::EndingCalibration(my_context ctx) : my_base(ctx)
{            
    PRINTENGINE->SendStatus(EndingCalibrationState, Entering); 
    
    // check to see if the door is still open 
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

EndingCalibration::~EndingCalibration()
{
    PRINTENGINE->SendStatus(EndingCalibrationState, Leaving); 
}

sc::result EndingCalibration::react(const EvAtHome&)
{  
    return transit<Home>();
}   

Registering::Registering(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(RegisteringState, Entering);  
}
 
Registering::~Registering()
{
    PRINTENGINE->SendStatus(RegisteringState, Leaving); 
}

sc::result Registering::react(const EvLeftButton&) 
{
    return transit<Home>();
}

sc::result Registering::react(const EvRegistered&)  
{
    return transit<Home>();
}
  
ConfirmCancel::ConfirmCancel(my_context ctx): 
my_base(ctx),
_separated(false)
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Entering);  
}

ConfirmCancel::~ConfirmCancel()
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Leaving);      
}

sc::result ConfirmCancel::react(const EvCancel&)    
{    
    if(context<PrinterStateMachine>().IsMotorMoving())
    {
        // don't allow cancellation while motors are still moving
        return discard_event();
    }
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
}

sc::result ConfirmCancel::react(const EvRightButton&)    
{    
    post_event(EvNoCancel());
    return discard_event();
}

sc::result ConfirmCancel::react(const EvNoCancel&)    
{  
     if(_separated)
        return transit<Exposing>();
    else
        return transit<sc::deep_history<Exposing> >();
}

sc::result ConfirmCancel::react(const EvLeftButton&)    
{    
    post_event(EvCancel());
    return discard_event();   
}

sc::result ConfirmCancel::react(const EvSeparated&)
{
    // since we separated while awaiting cancel confirmation, 
    // we'll want to go to the next appropriate state if we resume
     _separated = true;
}

Home::Home(my_context ctx) : my_base(ctx)
{
    // get the UI sub-state so that we'll display the appropriate screen
    UISubState subState = PRINTENGINE->GetHomeUISubState();
    if(subState == NoUISubState)
        subState = PRINTENGINE->HasAtLeastOneLayer() ? HavePrintData : 
                                                       NoPrintData;
    PRINTENGINE->SendStatus(HomeState, Entering, subState); 
    
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

sc::result Home::react(const EvRightButton&)
{
    switch(PRINTENGINE->GetUISubState())
    {
        case NoPrintData:
        case LoadingPrintData:
            // ignore button press when nothing to print or loading data
            return discard_event(); 
            break;
         
        case Registered:
            PRINTENGINE->ClearHomeUISubState(); // user pressed OK
        case PrintDataLoadFailed:
            // just refresh the home screen with the appropriate message
            PRINTENGINE->SendStatus(HomeState, NoChange, 
                 PRINTENGINE->HasAtLeastOneLayer() ? HavePrintData : 
                                                     NoPrintData); 
            return discard_event(); 
            break;

        default:
            return TryStartPrint();
            break;
    }
}

sc::result Home::react(const EvLeftButton&)
{
    if(PRINTENGINE->HasAtLeastOneLayer())
    {
        PRINTENGINE->ClearPrintData();
        // refresh the home screen to show no more print data
        PRINTENGINE->SendStatus(HomeState, NoChange, NoPrintData);
    }
    return discard_event(); 
}

sc::result Home::react(const EvRightButtonHold&)
{
    post_event(EvStartCalibration());
    return discard_event();    
}

sc::result Home::react(const EvStartCalibration&)
{
    PRINTENGINE->ClearHomeUISubState();
    return transit<Calibrate>();  
}

sc::result Home::react(const EvLeftButtonHold&)
{
    post_event(EvShowVersion());
    return discard_event(); 
}

sc::result Home::react(const EvConnected&)
{
    return transit<Registering>();
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
    // send the move to start position command to the motor board, and
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

PrintingLayer::PrintingLayer(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PrintingLayerState, Entering);
}

PrintingLayer::~PrintingLayer()
{
    PRINTENGINE->SendStatus(PrintingLayerState, Leaving);
}

sc::result PrintingLayer::react(const EvPause&)
{
    return transit<Paused>();
}

sc::result PrintingLayer::react(const EvRightButton&)
{
    post_event(EvPause());
    return discard_event();         
}

sc::result PrintingLayer::react(const EvLeftButton&)
{
    return transit<ConfirmCancel>();    
}

Paused::Paused(my_context ctx) : 
my_base(ctx),
_separated(false)
{
    PRINTENGINE->SendStatus(PausedState, Entering, 
                context<PrinterStateMachine>()._pausedSubState);
}

Paused::~Paused()
{
    context<PrinterStateMachine>()._pausedSubState = NoUISubState;
    
    PRINTENGINE->SendStatus(PausedState, Leaving);
}

sc::result Paused::react(const EvResume&)
{  
    if(_separated)
        return transit<Exposing>();
    else
        return transit<sc::deep_history<Exposing> >();
}

sc::result Paused::react(const EvRightButton&)
{
    post_event(EvResume());
    return discard_event();         
}

sc::result Paused::react(const EvLeftButton&)
{
    return transit<ConfirmCancel>();    
}

sc::result Paused::react(const EvSeparated&)
{
    // since we separated while paused, 
    // we'll want to go to the next appropriate state if we resume
     _separated = true;
}

sc::result Paused::react(const EvCancel&)    
{    
    if(context<PrinterStateMachine>().IsMotorMoving())
    {
        // don't allow cancellation while motors are still moving
        return discard_event();
    }
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
}

double Exposing::_remainingExposureTimeSec = 0.0;
int Exposing::_previousLayer = 0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{
#ifdef DEBUG
    // for comparing actual layer times against estimates    
//    std::cout << "last layer took (ms)" << StopStopwatch() << std::endl;
//    StartStopwatch();
#endif   
    if(PRINTENGINE->CancelRequested())
    {
        post_event(EvCancel());
        return;
    }
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
        if(!PRINTENGINE->NextLayer())
            return;  // unable to load image for next layer
        
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
    if(_remainingExposureTimeSec > 0.0)
    {
        _previousLayer = PRINTENGINE->GetCurrentLayer();
    }
    PRINTENGINE->SendStatus(ExposingState, Leaving);
}

sc::result Exposing::react(const EvExposed&)
{
    return transit<Separating>();
}

sc::result Exposing::react(const EvCancel&)    
{    
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
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
    
    PRINTENGINE->ClearRotationInterrupt();
    
    // send the appropriate separation command to the motor board, and
    // record the motor board event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(
                               PRINTENGINE->GetSeparationCommand(), Separated);
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus(SeparatingState, Leaving);
}

sc::result Separating::react(const EvSeparated&)
{
    if(!PRINTENGINE->GotRotationInterrupt())
    {
        // we didn't get the expected interrupt from the rotation sensor, 
        // so the resin tray must have jammed
        
        // set the UI substate to show a special message when paused
        context<PrinterStateMachine>()._pausedSubState = RotationJammed;
        return transit<Paused>();
    }
    if(PRINTENGINE->NoMoreLayers())
        return transit<EndingPrint>();
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
    PRINTENGINE->ClearCurrentPrint();  
    PRINTENGINE->SendStatus(EndingPrintState, Leaving);  
}

sc::result EndingPrint::react(const EvPrintEnded&)
{
    context<PrinterStateMachine>()._homingSubState = PrintCompleted;
    return transit<Homing>();
}

