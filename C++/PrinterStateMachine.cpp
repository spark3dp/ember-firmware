//  File:   PrinterStateMachine.cpp
//  Implements all classes used by the PrintEngine's state machine
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <fcntl.h>

#include <PrinterStateMachine.h>
#include <PrintEngine.h>
#include <Hardware.h>
#include <Logger.h>
#include <utils.h>
#include <Settings.h>
#include <MessageStrings.h>
#include <Filenames.h>
#include <ImageProcessor.h>

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_isProcessing(false),
_homingSubState(NoUISubState),
_remainingUnjamTries(0),
_motionCompleted(false),
_skipHomingRotation(false)
{
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
    terminate(); 
}

// Sends the given command to the motor.
void PrinterStateMachine::SendMotorCommand(HighLevelMotorCommand command)
{
    _motionCompleted = false;
    // send the command to the motor controller
    _pPrintEngine->SendMotorCommand(command);  
}

// Handle completion (or failure) of motor command)
void PrinterStateMachine::MotionCompleted(bool successfully)
{    
    if (!successfully)
        return;     // we've already handled the error, so nothing more to do
    
    // this flag allows us to handle the event in the rare case that the motion 
    // completed just after a pause was requested on entry to DoorOpen or
    // ConfirmCancel states
    _motionCompleted = true;
    
    process_event(EvMotionCompleted());
}

// Overrides (hides) base type behavior by flagging when we are in the middle
// of processing.
void PrinterStateMachine::process_event(const event_base_type& evt)
{
    _isProcessing = true;
    sc::state_machine< PrinterStateMachine, PrinterOn >::process_event(evt);
    _isProcessing = false;    
}

// Handle an error that prevents printing or moving the motors, by going to the 
// Error state.
void PrinterStateMachine::HandleFatalError()
{
    // we can only call process_event if we aren't already processing an event
    if (_isProcessing)
        post_event(EvError());
    else
        process_event(EvError());
}

// Perform common actions needed when canceling a print in progress.
void PrinterStateMachine::CancelPrint()
{
    _motionCompleted = false;
    _pPrintEngine->ClearCurrentPrint(true);
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

sc::result PrinterOn::react(const EvError&)
{
    return transit<Error>();
}

sc::result PrinterOn::react(const EvCancel&)    
{   
    if (PRINTENGINE->PrintIsInProgress())
    {
        context<PrinterStateMachine>().CancelPrint();

        return transit<AwaitingCancelation>();
    }
    else
        return discard_event(); 
}

AwaitingCancelation::AwaitingCancelation(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(AwaitingCancelationState, Entering);
    
    // check to see if the door is open after canceling from calibrating
    if (PRINTENGINE->DoorIsOpen())
        post_event(EvDoorOpened());
    else if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

AwaitingCancelation::~AwaitingCancelation()
{
    PRINTENGINE->SendStatus(AwaitingCancelationState, Leaving);
}

sc::result AwaitingCancelation::react(const EvMotionCompleted&)
{
    if (context<PrinterStateMachine>()._skipHomingRotation)
        context<PrinterStateMachine>().SendMotorCommand(GoHomeWithoutRotateHome);
    else 
        context<PrinterStateMachine>().SendMotorCommand(GoHome);

    return transit<Homing>();
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

sc::result ShowingVersion::react(const EvLeftButton&)
{
    if(PRINTENGINE->CanUpgradeProjector())
        return transit<ConfirmUpgrade>(); 
    else
        return discard_event();  
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
    
    // see if the printer should be put into demo mode
    if (PRINTENGINE->DemoModeRequested())
        post_event(EvEnterDemoMode()); 
    
    // check to see if the door is open on startup
    if (PRINTENGINE->DoorIsOpen())
    {
        post_event(EvDoorOpened());
    }
    else
    {
        PRINTENGINE->Initialize();
        post_event(EvInitialized());
    }
}

Initializing::~Initializing()
{
    PRINTENGINE->SendStatus(InitializingState, Leaving); 
}

sc::result Initializing::react(const EvInitialized&)
{
    context<PrinterStateMachine>().SendMotorCommand(GoHome);
    return transit<Homing>();
}

sc::result Initializing::react(const EvEnterDemoMode&)
{
    return transit<DemoMode>();
}

DoorOpen::DoorOpen(my_context ctx) : 
my_base(ctx),
_attemptedUnjam(false)
{
    // if we were in the middle of downloading data, allow it to continue
    UISubState subState = NoUISubState;
    if (PRINTENGINE->GetHomeUISubState() == DownloadingPrintData)
        subState = DownloadingPrintData;
    
    PRINTENGINE->SendStatus(DoorOpenState, Entering, subState); 
    
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
    PRINTENGINE->SendStatus(DoorOpenState, NoChange, ClearingScreen); 
    
    return transit<sc::deep_history<Initializing> >();
}

sc::result DoorOpen::react(const EvRightButton&)
{
    switch(PRINTENGINE->GetUISubState())
    {
        case PrintDataLoadFailed:
        case PrintDownloadFailed:
            // user pressed OK after showing error message
            // clear the home UI substate,
            PRINTENGINE->ClearHomeUISubState();
            // and show the normal oorOpen screen
            PRINTENGINE->SendStatus(DoorOpenState, NoChange, NoUISubState);
            break;
            
        default:
            // random press of right button, do nothing
            break;
    }
    return discard_event(); 
}

Homing::Homing(my_context ctx) : my_base(ctx)
{            
    PRINTENGINE->SendStatus(HomingState, Entering, 
                            context<PrinterStateMachine>()._homingSubState);
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

Homing::~Homing()
{
    PRINTENGINE->SendStatus(HomingState, Leaving); 
}

sc::result Homing::react(const EvMotionCompleted&)
{
    context<PrinterStateMachine>()._homingSubState = NoUISubState;
    
    // previous job ID no longer applies
    PRINTENGINE->ClearJobID();
    // nor does the rating for the previous print
    PRINTENGINE->SetPrintFeedback(Unknown);
    
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

sc::result Error::react(const EvLeftButton&)
{   
    post_event(EvReset());
    return discard_event();
}

sc::result Error::react(const EvLeftButtonHold&)
{
    return transit<ShowingVersion>();
}

Calibrating::Calibrating(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(CalibratingState, Entering);     
}
   
Calibrating::~Calibrating()
{
    PRINTENGINE->SendStatus(CalibratingState, Leaving);         
}

sc::result Calibrating::react(const EvLeftButton&)
{
    context<PrinterStateMachine>()._homingSubState = NoUISubState;
    post_event(EvCancel());
    return discard_event(); 
}   

sc::result Calibrating::react(const EvRightButton&)
{
    return transit<InitializingLayer>();
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
bool ConfirmCancel::_fromJammedOrUnjamming = false;

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

sc::result ConfirmCancel::react(const EvRightButton&)    
{    
    post_event(EvResume());
    return discard_event();
}

sc::result ConfirmCancel::react(const EvResume&)    
{  
    if (_fromPaused)
    {
        context<PrinterStateMachine>().SendMotorCommand(ResumeFromInspect); 
        _fromPaused = false;
        return transit<MovingToResume>();
    }
    else if (_fromJammedOrUnjamming)
    {
        // clear any motor command that was in progress
        PRINTENGINE->ClearPendingMovement();
        
        // rotate to a known position before the approach
        context<PrinterStateMachine>().SendMotorCommand(ApproachAfterJam);
        _fromJammedOrUnjamming = false;
        return transit<Approaching>();
    }
    else
    {
        PRINTENGINE->ResumeMovement();
        return transit<sc::deep_history<Pressing> >();
    }
}

sc::result ConfirmCancel::react(const EvLeftButton&)    
{   
    context<PrinterStateMachine>()._homingSubState = PrintCanceled;
    post_event(EvCancel());
    return discard_event();   
}

Home::Home(my_context ctx) : my_base(ctx)
{
    // get the UI sub-state so that we'll display the appropriate screen
    UISubState subState = PRINTENGINE->GetHomeUISubState();
    if (subState == NoUISubState)
        subState = PRINTENGINE->HasAtLeastOneLayer() ? HavePrintData : 
                                                       NoPrintData;
    
    PRINTENGINE->SetCanLoadPrintData(subState != LoadingPrintData &&
                                     subState != DownloadingPrintData);
    
    PRINTENGINE->SendStatus(HomeState, Entering, subState); 
    
    // the timeout timer should already have been cleared, but this won't hurt
    PRINTENGINE->ClearMotorTimeoutTimer();
    
    // disengage the motors when we're in the home position
    PRINTENGINE->DisableMotors();
}

Home::~Home()
{
    PRINTENGINE->SendStatus(HomeState, Leaving); 
}

sc::result Home::TryStartPrint()
{
    context<PrinterStateMachine>()._skipHomingRotation = false;
    
    if (PRINTENGINE->TryStartPrint())
    {
        // send the move to start position command to the motor controller
        context<PrinterStateMachine>().SendMotorCommand(MoveToStartPosition);

        PRINTENGINE->SetCanLoadPrintData(false);
        return transit<MovingToStartPosition>();
    }
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
        case USBDriveError:
            PRINTENGINE->ClearHomeUISubState(); // user pressed OK
        case PrintDataLoadFailed:
        case PrintDownloadFailed:
            // just refresh the home screen with the appropriate message
            PRINTENGINE->ShowScreenFor(PRINTENGINE->HasAtLeastOneLayer() ? 
                                                HavePrintData : NoPrintData); 
            return discard_event(); 
            break;
            
        case USBDriveFileFound:
            PRINTENGINE->LoadPrintFileFromUSBDrive();
            return discard_event();
            break;

        default:
            return TryStartPrint();
            break;
    }
}

sc::result Home::react(const EvLeftButton&)
{
    switch(PRINTENGINE->GetUISubState())
    {
        case HavePrintData:
        case LoadedPrintData:
            if (PRINTENGINE->HasAtLeastOneLayer())
                PRINTENGINE->ClearPrintData();
        case USBDriveFileFound:
            // refresh the home screen with the appropriate message
            PRINTENGINE->SendStatus(HomeState, NoChange, 
                 PRINTENGINE->HasAtLeastOneLayer() ? HavePrintData : 
                                                     NoPrintData); 
            break;
    }
    return discard_event(); 
}

sc::result Home::react(const EvLeftButtonHold&)
{
    PRINTENGINE->SetCanLoadPrintData(false);
    return transit<ShowingVersion>();
}

sc::result Home::react(const EvConnected&)
{
    PRINTENGINE->SetCanLoadPrintData(false);
    return transit<Registering>();
}

MovingToPause::MovingToPause(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToPauseState, Entering);

    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

MovingToPause::~MovingToPause()
{
    PRINTENGINE->SendStatus(MovingToPauseState, Leaving); 
}

sc::result MovingToPause::react(const EvMotionCompleted&)
{
        return transit<Paused>();
}

MovingToResume::MovingToResume(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToResumeState, Entering);

    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());  
}

MovingToResume::~MovingToResume()
{
    PRINTENGINE->SendStatus(MovingToResumeState, Leaving); 
}

sc::result MovingToResume::react(const EvMotionCompleted&)
{
    return transit<InitializingLayer>();
}

MovingToStartPosition::MovingToStartPosition(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Entering,
               PRINTENGINE->SkipCalibration() ? NoUISubState : CalibratePrompt);
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

MovingToStartPosition::~MovingToStartPosition()
{
    PRINTENGINE->SendStatus(MovingToStartPositionState, Leaving);
}

sc::result MovingToStartPosition::react(const EvMotionCompleted&)
{    
    if (PRINTENGINE->SkipCalibration())
        return transit<InitializingLayer>();
    else
        return transit<Calibrating>();
}

sc::result MovingToStartPosition::react(const EvLeftButton&)
{
    context<PrinterStateMachine>()._homingSubState = NoUISubState;
    post_event(EvCancel());
    return discard_event();  
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
    if (PRINTENGINE->PauseRequested())
        return discard_event();
    else
    {
        ConfirmCancel::_fromPaused = false;
        ConfirmCancel::_fromJammedOrUnjamming = false;
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
    context<PrinterStateMachine>().SendMotorCommand(ResumeFromInspect); 
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
    ConfirmCancel::_fromJammedOrUnjamming = false;

    return transit<ConfirmCancel>();    
}

Unjamming::Unjamming(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(UnjammingState, Entering);
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

Unjamming::~Unjamming()
{
    PRINTENGINE->SendStatus(UnjammingState, Leaving);
}

sc::result Unjamming::react(const EvMotionCompleted&)
{  
    if (PRINTENGINE->GotRotationInterrupt()) 
    {  
        // we successfully unjammed
        context<PrinterStateMachine>().SendMotorCommand(Approach);

        return transit<Approaching>();
    }
    else if (--context<PrinterStateMachine>()._remainingUnjamTries > 0)
    {
        context<PrinterStateMachine>().SendMotorCommand(RecoverFromJam);
        
        return discard_event();  
    }
    else
        return transit<Jammed>();
}

sc::result Unjamming::react(const EvLeftButton&)
{
    PRINTENGINE->PauseMovement();
    
    // if the user doesn't confirm the cancellation, 
    // we can resume to the Approaching state
    ConfirmCancel::_fromJammedOrUnjamming = true;
    ConfirmCancel::_fromPaused = false;
    return transit<ConfirmCancel>();    
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
    // rotate to a known position before the approach
    context<PrinterStateMachine>().SendMotorCommand(ApproachAfterJam);

    return transit<Approaching>();
}

sc::result Jammed::react(const EvRightButton&)
{
    post_event(EvResume());
    return discard_event();         
}

sc::result Jammed::react(const EvLeftButton&)
{
    ConfirmCancel::_fromJammedOrUnjamming = true;
    ConfirmCancel::_fromPaused = false;
    return transit<ConfirmCancel>();    
}

InitializingLayer::InitializingLayer(my_context ctx) : my_base(ctx)
{    
    // check to see if the door is still open after calibrating
    if (PRINTENGINE->DoorIsOpen())
        post_event(EvDoorOpened());
    else
    {
        // perform initialization needed for next layer
        // (even if the door is opened and closed again while here,
        // this won't be called more than once per layer, because we're going to 
        // immediately transition to the next state)
        PRINTENGINE->NextLayer();
        post_event(EvInitialized());
        
        // rotation homing not wanted on cancellation (till end of Exposing) 
        context<PrinterStateMachine>()._skipHomingRotation = true;
    }
    
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    // don't send status till after estimated print time has been set
    PRINTENGINE->SendStatus(InitializingLayerState, Entering, uiSubState);
}

InitializingLayer::~InitializingLayer()
{
    PRINTENGINE->SendStatus(InitializingLayerState, Leaving);
}

sc::result InitializingLayer::react(const EvInitialized&)
{
    if (PRINTENGINE->GetTrayDeflection() != 0)
    {
        // begin with tray deflection
        context<PrinterStateMachine>().SendMotorCommand(Press);
        return transit<Pressing>();
    }
    else if(PRINTENGINE->NeedsPreExposureDelay())
    {
        // begin with pre-exposure delay
        return transit<PreExposureDelay>();
    }        
    else
    {
        // begin with exposing
        return transit<Exposing>();
    }
}

Pressing::Pressing(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PressingState, Entering, uiSubState);
    
    if (context<PrinterStateMachine>()._motionCompleted) 
        post_event(EvMotionCompleted());
}

Pressing::~Pressing()
{
    PRINTENGINE->SendStatus(PressingState, Leaving);
}

sc::result Pressing::react(const EvMotionCompleted&)
{
    if (!PRINTENGINE->NeedsTrayDeflectionPause())
    {
        // we can skip the delay state
        context<PrinterStateMachine>().SendMotorCommand(UnPress);
        return transit<Unpressing>();
    }
    else
    {
        return transit<PressDelay>();
    }
}

PressDelay::PressDelay(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PressDelayState, Entering, uiSubState);
    
    PRINTENGINE->StartDelayTimer(PRINTENGINE->GetTrayDeflectionPauseTimeSec());
}

PressDelay::~PressDelay()
{
    PRINTENGINE->SendStatus(PressDelayState, Leaving);
}

sc::result PressDelay::react(const EvDelayEnded&) 
{
    context<PrinterStateMachine>().SendMotorCommand(UnPress);

    return transit<Unpressing>();
}

Unpressing::Unpressing(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(UnpressingState, Entering, uiSubState);
    
    if (context<PrinterStateMachine>()._motionCompleted) 
        post_event(EvMotionCompleted());
}

Unpressing::~Unpressing()
{
    PRINTENGINE->SendStatus(UnpressingState, Leaving);
}

sc::result Unpressing::react(const EvMotionCompleted&)
{
    if(PRINTENGINE->NeedsPreExposureDelay())
        return transit<PreExposureDelay>();
    else
        return transit<Exposing>();
}

PreExposureDelay::PreExposureDelay(my_context ctx) : my_base(ctx)
{  
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PreExposureDelayState, Entering, uiSubState);

    if (PRINTENGINE->NeedsPreExposureDelay())
    {
        PRINTENGINE->StartDelayTimer(PRINTENGINE->GetPreExposureDelayTimeSec());
    }
    else
    {
        // no delay needed (we really should never get here)
        post_event(EvDelayEnded());
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

double Exposing::_remainingExposureTimeSec = 0.0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{   
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(ExposingState, Entering, uiSubState);
    
    // get remaining exposure time 
    double exposureTimeSec;
    if (_remainingExposureTimeSec > 0)
    {
        // we must be returning here after door opened or cancel unconfirmed
        exposureTimeSec = _remainingExposureTimeSec;
    }
    else
    { 
        // initial entry into constructor for exposing this layer
        if(!PRINTENGINE->AwaitEndOfBackgroundThread())
            return;  // fatal error 
        
        exposureTimeSec = PRINTENGINE->GetExposureTimeSec();
    }
      
    // display current layer
    PRINTENGINE->ShowImage();
    
    PRINTENGINE->StartExposureTimer(exposureTimeSec);
}

Exposing::~Exposing()
{
    // black out the projected image
    PRINTENGINE->TurnProjectorOff();
    
    // if we're leaving during the middle of exposure, 
    // we need to record that fact, 
    // as well as our layer and the remaining exposure time
    _remainingExposureTimeSec = PRINTENGINE->GetRemainingExposureTimeSec();

    PRINTENGINE->SendStatus(ExposingState, Leaving);
}

sc::result Exposing::react(const EvExposed&)
{
    // load and process the image for the next layer, if there is one
    if(PRINTENGINE->MoreLayers())
    {
        if (!PRINTENGINE->LoadNextLayerImage())
            return discard_event(); // fatal error already handled
    }
    
    PRINTENGINE->ClearRotationInterrupt();
    
    // send the separation command to the motor controller
    context<PrinterStateMachine>().SendMotorCommand(Separate);
    
    // rotation homing now needed on cancellation
    context<PrinterStateMachine>()._skipHomingRotation = false;

    return transit<Separating>();
}

// Clear the information saved when leaving Exposing before the exposure is 
// actually completed
void Exposing::ClearPendingExposureInfo()
{
    _remainingExposureTimeSec = 0;
}

Separating::Separating(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    PRINTENGINE->SendStatus(SeparatingState, Entering, uiSubState);
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

Separating::~Separating()
{
    PRINTENGINE->SendStatus(SeparatingState, Leaving);
}

sc::result Separating::react(const EvMotionCompleted&)
{
    if (!PRINTENGINE->GotRotationInterrupt())
    {
        // we didn't get the expected interrupt from the rotation sensor, 
        // so the resin tray must have jammed
            
        char msg[100];
        sprintf(msg, LOG_JAM_DETECTED, PRINTENGINE->GetCurrentLayerNum(),
                                       PRINTENGINE->GetTemperature());
        Logger::LogMessage(LOG_INFO, msg);
        
        context<PrinterStateMachine>()._remainingUnjamTries = 
                            PrinterSettings::Instance().GetInt(MAX_UNJAM_TRIES);
        
        if (context<PrinterStateMachine>()._remainingUnjamTries > 0)
        {
            context<PrinterStateMachine>().SendMotorCommand(RecoverFromJam);
            return transit<Unjamming>();
        }
        else
            return transit<Jammed>(); 
    }
    else
    {
        context<PrinterStateMachine>().SendMotorCommand(Approach);
        return transit<Approaching>();
    }
}

Approaching::Approaching(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    PRINTENGINE->SendStatus(ApproachingState, Entering, uiSubState);
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

Approaching::~Approaching()
{
    PRINTENGINE->SendStatus(ApproachingState, Leaving);
}

sc::result Approaching::react(const EvMotionCompleted&)
{
    if (!PRINTENGINE->MoreLayers())
    {
        PRINTENGINE->ClearCurrentPrint();
        context<PrinterStateMachine>()._homingSubState = PrintCompleted;
        context<PrinterStateMachine>().SendMotorCommand(GoHomeWithoutRotateHome);
        
        if (IsInternetConnected())
            return transit<GettingFeedback>(); 
        else
            return transit<Homing>();    
    }
    else if (PRINTENGINE->PauseRequested())
    {    
        PRINTENGINE->SetInspectionRequested(false);
        context<PrinterStateMachine>().SendMotorCommand(PauseAndInspect);
        return transit<MovingToPause>();
    }
    else
    {
        return transit<InitializingLayer>();
    }
}

GettingFeedback::GettingFeedback(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(GettingFeedbackState, Entering);
}

GettingFeedback::~GettingFeedback()
{
    PRINTENGINE->SendStatus(GettingFeedbackState, Leaving);
}

sc::result GettingFeedback::react(const EvLeftButton&)
{
    // indicate print failed
    PRINTENGINE->SetPrintFeedback(Failed);
    return transit<Homing>(); 
}

sc::result GettingFeedback::react(const EvRightButton&)
{
    // indicate print was successful
    PRINTENGINE->SetPrintFeedback(Succeeded);
    return transit<Homing>();          
}

sc::result GettingFeedback::react(const EvMotionCompleted&)
{
    // Since the build head is now in the physical home position,
    // (even though we're not yet in the Home state), disengage the motors. 
    PRINTENGINE->DisableMotors();
    return discard_event();
}

sc::result GettingFeedback::react(const EvDismiss&)
{
    // Dismiss the feedback screen, and leave the reported print_rating as
    // unknown, since feedback has presumably already been obtained elsewhere.
    return transit<Homing>(); 
}

DemoMode::DemoMode(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(DemoModeState, Entering);
    
    PRINTENGINE->SetDemoMode();
}

DemoMode::~DemoMode()
{
    PRINTENGINE->SendStatus(DemoModeState, Leaving);
}

ConfirmUpgrade::ConfirmUpgrade(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(ConfirmUpgradeState, Entering);
}

ConfirmUpgrade::~ConfirmUpgrade()
{
    PRINTENGINE->SendStatus(ConfirmUpgradeState, Leaving);
}

sc::result ConfirmUpgrade::react(const EvRightButton&)
{
    // clear the screen 
    PRINTENGINE->SendStatus(ConfirmUpgradeState, NoChange, ClearingScreen);
    // and start the upgrade process
    return transit<UpgradingProjector>();    
}

sc::result ConfirmUpgrade::react(const EvCancel&)
{
    return transit<ShowingVersion>();
}

sc::result ConfirmUpgrade::react(const EvReset&)
{
    PRINTENGINE->ClearCurrentPrint();  // probably not necessary, but can't hurt
    return transit<Initializing>();
}

sc::result ConfirmUpgrade::react(const EvLeftButton&)
{
    post_event(EvCancel());
    return discard_event();   
}

UpgradingProjector::UpgradingProjector(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(UpgradingProjectorState, Entering);
    PRINTENGINE->PutProjectorInProgramMode(true);
}

UpgradingProjector::~UpgradingProjector()
{
    PRINTENGINE->SendStatus(UpgradingProjectorState, Leaving);
    // the following doesn't actually work to get us out of Program Mode
    PRINTENGINE->PutProjectorInProgramMode(false);
}

sc::result UpgradingProjector::react(const EvDelayEnded&)
{
    // do actual re-programming of projector firmware
    PRINTENGINE->UpgradeProjectorFirmware();
    if(!PRINTENGINE->ProjectorProgrammingCompleted())
    {
        // minimal delay, to allow progress update, which itself takes 300 ms
        PRINTENGINE->StartDelayTimer(0.001);
        // send status, to update progress indicator
        PRINTENGINE->SendStatus(UpgradingProjectorState, NoChange);
    }
    return discard_event();
}

sc::result UpgradingProjector::react(const EvUpgadeCompleted&)
{
    // all done
    return transit<UpgradeComplete>();
}

sc::result UpgradingProjector::react(const EvError&)
{
    return transit<Error>();
}

UpgradeComplete::UpgradeComplete(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(UpgradeCompleteState, Entering);
}

UpgradeComplete::~UpgradeComplete()
{
    PRINTENGINE->SendStatus(UpgradeCompleteState, Leaving);
} 


#undef PRINTENGINE