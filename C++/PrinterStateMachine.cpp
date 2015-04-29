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
#include <Settings.h>
#include <MessageStrings.h>

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_pendingMotorEvent(None),
_isProcessing(false),
_homingSubState(NoUISubState),
_atInspectionPosition(false)        
{
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
}

/// Sends the given command to the motor, and sets the given motor event as
/// the one that's pending, and sets the motor timeout.
void PrinterStateMachine::SetMotorCommand(const char command, 
                                      PendingMotorEvent pending,
                                      int timeoutSec)
{
    // send the command to the motor controller
    PRINTENGINE->SendMotorCommand(command);
    // record the event to generate when the command is completed
    _pendingMotorEvent = pending;   
    // set the timeout 
    PRINTENGINE->StartMotorTimeoutTimer(timeoutSec);
}

/// Handle completion (or failure) of motor command)
/// Note: we need to clear the pending motor event just before processing new 
/// state machine events, in case those also want to send a new motor command 
void PrinterStateMachine::MotionCompleted(bool successfully)
{
    // clear the pending timeout
    PRINTENGINE->ClearMotorTimeoutTimer();

#ifdef DEBUG
//    std::cout << "Motion completed  " << 
//                (successfully ? "" : "un") <<
//                "successfully with pending event " <<
//                _pendingMotorEvent << std::endl;
#endif
    PendingMotorEvent origEvent = _pendingMotorEvent;
    _pendingMotorEvent = None;
    
    if(!successfully)
        return;     // we've already handled the error, so nothing more to do

    switch(origEvent)
    {
        case None:
            PRINTENGINE->HandleError(UnexpectedMotionEnd);
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

        case AtPauseAndInspect:
            // this flag indicates that we'll need to move the build head back 
            // down to resume, without relying on a second call to 
            // PrintEngine::CanInspect
            _atInspectionPosition = true;
            process_event(EvAtPause());
            break;

        case AtResume:
            process_event(EvAtResume());
            break;

        default:
            PRINTENGINE->HandleError(UnknownMotorEvent, false, NULL, 
                                     origEvent);
            break;
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

/// Perform common actions needed when canceling a print in progress.
void PrinterStateMachine::CancelPrint()
{
    PRINTENGINE->ClearCurrentPrint();
    _homingSubState = PrintCanceled;
    _pendingMotorEvent = None;
}

/// Perform actions required after separation and return the next state to which
/// the current state needs to transition.
PrintEngineState PrinterStateMachine::AfterSeparation()
{
    if(_pPrintEngine->NoMoreLayers())
    {
        _pPrintEngine->ClearCurrentPrint();
        _homingSubState = PrintCompleted;
        return HomingState;
    }
    else if(!_pPrintEngine->GotRotationInterrupt())
    {
        // we didn't get the expected interrupt from the rotation sensor, 
        // so the resin tray must have jammed
            
        char msg[100];
        sprintf(msg, LOG_JAM_DETECTED, _pPrintEngine->GetCurrentLayer(),
                                       _pPrintEngine->GetTemperature());
        LOGGER.LogMessage(LOG_INFO, msg);
            
        return JammedState;
    }
    else if(_pPrintEngine->PauseRequested())
    {    
        _pPrintEngine->SetInspectionRequested(false);
        return MovingToPauseState;
    }
    else
        return PreExposureDelayState;
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

sc::result ShowingVersion::react(const EvReset&)
{
    PRINTENGINE->ClearCurrentPrint();  // probably not necessary, but can't hurt
    return transit<Initializing>();
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
    
    post_event(EvInitialized());
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
_separated(false),
_atPause(false),
_atResume(false)       
{
    PRINTENGINE->SendStatus(DoorOpenState, Entering); 
    
    PRINTENGINE->PauseMovement();
}

DoorOpen::~DoorOpen()
{
    PRINTENGINE->SendStatus(DoorOpenState, Leaving); 
    
    PRINTENGINE->ResumeMovement();
}

sc::result DoorOpen::react(const EvDoorClosed&)
{
    // arrange to clear the screen first
    PRINTENGINE->SendStatus(DoorOpenState, NoChange, ExitingDoorOpen); 
    
    // even though we try to pause any movement in progress on entry to 
    // DoorOpen, it's still possible for a motion to complete just as we enter 
    // this state, before the pause could take effect
    if(_atStartPosition)
    {
        // we got to the start position when the door was open, 
        // so we need to either complete calibration or start exposing
        if(PRINTENGINE->SkipCalibration())
            return transit<PreExposureDelay>();
        else
            return transit<Calibrating>();
    }
    else if(_separated)
    {
        // we completed a separation when the door was open 
        switch(context<PrinterStateMachine>().AfterSeparation())
        {
            case HomingState:
                return transit<Homing>();
                break;
                
            case JammedState:
                return transit<Jammed>();
                break;

            case MovingToPauseState:
                return transit<MovingToPause>();
                break;

            case PreExposureDelayState:
                return transit<PreExposureDelay>();
                break;  
        }
    }
    else if(_atPause)
    {
        // we got to the pause position when the door was open
        return transit<Paused>();
    }      
    else if(_atResume)
    {
        // we got to the position for resuming printing when the door was open
        return transit<PreExposureDelay>();
    }    
    else
        return transit<sc::deep_history<Initializing> >();
}

/// The following DoorOpen reactions handle the completion of motor movements
/// while the door was open by setting flags that indicate what we need to do
/// when the door is closed, since we can't just use history in those cases. 
sc::result DoorOpen::react(const EvAtStartPosition&)
{
    _atStartPosition = true;
}

sc::result DoorOpen::react(const EvSeparated&)
{
     _separated = true;
}

sc::result DoorOpen::react(const EvAtPause&)
{
    _atPause = true;
}

sc::result DoorOpen::react(const EvAtResume&)
{
    _atResume = true;
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
        // send the Home command to the motor controller, and
        // record the motor controller event we're waiting for
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
    
    // if a print was in progress, we don't clear it till after the above status
    // update, so that the Spark job status can go to 'failed'
    PRINTENGINE->ClearCurrentPrint();

    // in case the timeout timer is still running, we don't need another error
    PRINTENGINE->ClearMotorTimeoutTimer();
}

Error::~Error()
{
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
    return transit<PreExposureDelay>();    
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
  
bool ConfirmCancel::_fromPaused = false;
bool ConfirmCancel::_fromJammed = false;
bool ConfirmCancel::_separated = false;

ConfirmCancel::ConfirmCancel(my_context ctx): 
my_base(ctx)
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Entering);  
 
    PRINTENGINE->PauseMovement();
}

ConfirmCancel::~ConfirmCancel()
{
    PRINTENGINE->SendStatus(ConfirmCancelState, Leaving); 
}

sc::result ConfirmCancel::react(const EvCancel&)    
{              
    _separated = false;
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
}

sc::result ConfirmCancel::react(const EvRightButton&)    
{    
    post_event(EvResume());
    return discard_event();
}

sc::result ConfirmCancel::react(const EvResume&)    
{  
    if(_fromPaused)
        return transit<MovingToResume>();
    else if(_fromJammed)
        return transit<PreExposureDelay>();
    else if(_separated)
    {
        // this is still a possibility, if separation completed just as
        // PauseMovement was being called 
        _separated = false;
        switch(context<PrinterStateMachine>().AfterSeparation())
        {
            case HomingState:
                return transit<Homing>();
                break;
                
            case JammedState:
                return transit<Jammed>();
                break;
                
            case MovingToPauseState:
                return transit<MovingToPause>();
                break;

            default:
                return transit<PreExposureDelay>();
                break;  
        }
    }
    else
    {
        PRINTENGINE->ResumeMovement();
        return transit<sc::deep_history<PreExposureDelay> >();
    }
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
        return transit<MovingToStartPosition>();
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
        case DownloadingPrintData:
            // ignore button press when nothing to print or loading data
            return discard_event(); 
            break;
         
        case Registered:
        case WiFiConnecting:
        case WiFiConnectionFailed:
        case WiFiConnected:
            PRINTENGINE->ClearHomeUISubState(); // user pressed OK
        case PrintDataLoadFailed:
        case PrintDownloadFailed:
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

sc::result Home::react(const EvLeftButtonHold&)
{
    post_event(EvShowVersion());
    return discard_event(); 
}

sc::result Home::react(const EvConnected&)
{
    return transit<Registering>();
}

MovingToPause::MovingToPause(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToPauseState, Entering);

    if(PRINTENGINE->CanInspect())
    {
        context<PrinterStateMachine>().SetMotorCommand(
                                  PAUSE_AND_INSPECT_COMMAND, AtPauseAndInspect);
    }
    else    // no headroom for lifting, so we can just go to Paused state
        post_event(EvAtPause());
}

MovingToPause::~MovingToPause()
{
    PRINTENGINE->SendStatus(MovingToPauseState, Leaving); 
}

sc::result MovingToPause::react(const EvAtPause&)
{
        return transit<Paused>();
}

MovingToResume::MovingToResume(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToResumeState, Entering);

    if(context<PrinterStateMachine>()._atInspectionPosition)
    {
        context<PrinterStateMachine>()._atInspectionPosition = false;
        context<PrinterStateMachine>().SetMotorCommand(
                                       RESUME_FROM_INSPECT_COMMAND, AtResume); 
    }
    else    // we hadn't lifted, so no need to move down
        post_event(EvAtResume());
}

MovingToResume::~MovingToResume()
{
    PRINTENGINE->SendStatus(MovingToResumeState, Leaving); 
}

sc::result MovingToResume::react(const EvAtResume&)
{
    return transit<PreExposureDelay>();
}

MovingToStartPosition::MovingToStartPosition(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Entering,
               PRINTENGINE->SkipCalibration() ? NoUISubState : CalibratePrompt);
    // send the move to start position command to the motor controller, and
    // record the motor controller event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(MOVE_TO_START_POSN_COMMAND, 
                                   AtStartPosition, LONGEST_MOTOR_TIMEOUT_SEC);
}

MovingToStartPosition::~MovingToStartPosition()
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Leaving);
}

sc::result MovingToStartPosition::react(const EvAtStartPosition&)
{
    if(PRINTENGINE->SkipCalibration())
        return transit<PreExposureDelay>();
    else
        return transit<Calibrating>();
}

sc::result MovingToStartPosition::react(const EvRightButton&)
{
    PRINTENGINE->SetSkipCalibration();
    PRINTENGINE->SendStatus(MovingToStartPositionState);
}

PrintingLayer::PrintingLayer(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(PrintingLayerState, Entering);
}

PrintingLayer::~PrintingLayer()
{
    PRINTENGINE->SendStatus(PrintingLayerState, Leaving);
}

sc::result PrintingLayer::react(const EvRequestPause&)
{
    PRINTENGINE->SetInspectionRequested(true); 
    return discard_event();
}

sc::result PrintingLayer::react(const EvRightButton&)
{
    post_event(EvRequestPause());
    return discard_event();         
}

sc::result PrintingLayer::react(const EvLeftButton&)
{
    if(PRINTENGINE->PauseRequested())
        return discard_event();
    else
    {
        ConfirmCancel::_fromPaused = false;
        ConfirmCancel::_fromJammed = false;
        return transit<ConfirmCancel>();  
    }
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
    return transit<MovingToResume>();
}

sc::result Paused::react(const EvRightButton&)
{
    post_event(EvResume());
    return discard_event();         
}

sc::result Paused::react(const EvLeftButton&)
{
    ConfirmCancel::_fromPaused = true;
    return transit<ConfirmCancel>();    
}

sc::result Paused::react(const EvCancel&)    
{    
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
}

Jammed::Jammed(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(JammedState, Entering);
}

Jammed::~Jammed()
{
    PRINTENGINE->SendStatus(JammedState, Leaving);
}

sc::result Jammed::react(const EvResume&)
{  
    return transit<PreExposureDelay>();
}

sc::result Jammed::react(const EvRightButton&)
{
    post_event(EvResume());
    return discard_event();         
}

sc::result Jammed::react(const EvLeftButton&)
{
    ConfirmCancel::_fromJammed = true;
    return transit<ConfirmCancel>();    
}

sc::result Jammed::react(const EvCancel&)    
{    
    context<PrinterStateMachine>().CancelPrint();
    return transit<Homing>();
}

PreExposureDelay::PreExposureDelay(my_context ctx) : my_base(ctx)
{  
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PreExposureDelayState, Entering, uiSubState);

    // check to see if the door is still open after calibrating
    if(PRINTENGINE->DoorIsOpen())
        post_event(EvDoorOpened());
    else if(PRINTENGINE->CancelRequested())
        post_event(EvCancel());
    else
    {
        double delay = PRINTENGINE->GetPreExposureDelayTimeSec();
        if(delay <= 0.0)
        {
            // no delay needed
            post_event(EvDelayEnded());
        }
        else
            PRINTENGINE->StartPreExposureDelayTimer(delay);
    }
}

PreExposureDelay::~PreExposureDelay()
{    
    PRINTENGINE->SendStatus(PreExposureDelayState, Leaving);
}

sc::result PreExposureDelay::react(const EvDelayEnded&)
{    
    return transit<Exposing>();
}

sc::result PreExposureDelay::react(const EvCancel&)    
{    
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
      
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    // this update will convey the remaining print time to UI components
    PRINTENGINE->SendStatus(ExposingState, Entering, uiSubState);

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
    PRINTENGINE->ClearRotationInterrupt();
    
    // send the appropriate separation command to the motor controller, and
    // record the motor controller event we're waiting for
    context<PrinterStateMachine>().SetMotorCommand(
                               PRINTENGINE->GetSeparationCommand(), Separated,
                               PRINTENGINE->GetSeparationTimeoutSec());

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
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    PRINTENGINE->SendStatus(SeparatingState, Entering, uiSubState);
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus(SeparatingState, Leaving);
}

sc::result Separating::react(const EvSeparated&)
{
    switch(context<PrinterStateMachine>().AfterSeparation())
    {
        case HomingState:
            return transit<Homing>();
            break;
        
        case JammedState:
            return transit<Jammed>();
            break;
            
        case MovingToPauseState:
            return transit<MovingToPause>();
            break;
            
        case PreExposureDelayState:
            return transit<PreExposureDelay>();
            break;
    }
}


