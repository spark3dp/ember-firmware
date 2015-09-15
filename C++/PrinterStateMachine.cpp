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

#define PRINTENGINE context<PrinterStateMachine>().GetPrintEngine()

PrinterStateMachine::PrinterStateMachine(PrintEngine* pPrintEngine) :
_isProcessing(false),
_homingSubState(NoUISubState),
_remainingUnjamTries(0),
_motionCompleted(false)
{
    _pPrintEngine = pPrintEngine;
}

PrinterStateMachine::~PrinterStateMachine()
{
    terminate(); 
}

// Sends the given command to the motor.
void PrinterStateMachine::SendMotorCommand(const char command)
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
void PrinterStateMachine::process_event(const event_base_type & evt)
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
    _homingSubState = PrintCanceled;
}

// Send the command to the motor controller that moves to the home position.
void PrinterStateMachine::SendHomeCommand()
{
    // send the Home command to the motor controller
    SendMotorCommand(HOME_COMMAND);
}

// Get all the settings to be used for this layer, and send the tray deflection 
// command if a non-zero deflection has been requested; 
bool PrinterStateMachine::HandlePressCommand()
{
    _pPrintEngine->GetCurrentLayerSettings();
    if (_pPrintEngine->GetTrayDeflection() == 0)
        return false;
    
    SendMotorCommand(PRESS_COMMAND);
    return true;
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
    
    if (context<PrinterStateMachine>()._motionCompleted)
        post_event(EvMotionCompleted());
}

AwaitingCancelation::~AwaitingCancelation()
{
    PRINTENGINE->SendStatus(AwaitingCancelationState, Leaving);
}

sc::result AwaitingCancelation::react(const EvMotionCompleted&)
{
    context<PrinterStateMachine>().SendHomeCommand();
    
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
    context<PrinterStateMachine>().SendHomeCommand();
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
    
    return transit<sc::deep_history<Initializing> >();
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

sc::result Calibrating::react(const EvRightButton&)
{
    if (context<PrinterStateMachine>().HandlePressCommand())
        return transit<Pressing>(); 
    else
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
        context<PrinterStateMachine>().SendMotorCommand(
                                                   RESUME_FROM_INSPECT_COMMAND); 
        _fromPaused = false;
        return transit<MovingToResume>();
    }
    else if (_fromJammedOrUnjamming)
    {
        // clear any motor command that was in progress
        PRINTENGINE->ClearPendingMovement();
        
        // rotate to a known position before the approach
        context<PrinterStateMachine>().SendMotorCommand(
                                                    APPROACH_AFTER_JAM_COMMAND);
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
    if (PRINTENGINE->TryStartPrint())
    {
        // send the move to start position command to the motor controller, and
        // record the motor controller event we're waiting for
        context<PrinterStateMachine>().SendMotorCommand(
                                                    MOVE_TO_START_POSN_COMMAND);

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
            PRINTENGINE->SendStatus(HomeState, NoChange, 
                 PRINTENGINE->HasAtLeastOneLayer() ? HavePrintData : 
                                                     NoPrintData); 
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
    return transit<ShowingVersion>();
}

sc::result Home::react(const EvConnected&)
{
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
    if (context<PrinterStateMachine>().HandlePressCommand())
        return transit<Pressing>(); 
    else
        return transit<PreExposureDelay>();
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
    {
        if (context<PrinterStateMachine>().HandlePressCommand())
            return transit<Pressing>(); 
        else
            return transit<PreExposureDelay>();
    }
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
    context<PrinterStateMachine>().SendMotorCommand(
                                                   RESUME_FROM_INSPECT_COMMAND); 
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
        context<PrinterStateMachine>().SendMotorCommand(APPROACH_COMMAND);

        return transit<Approaching>();
    }
    else if (--context<PrinterStateMachine>()._remainingUnjamTries > 0)
    {
        context<PrinterStateMachine>().SendMotorCommand(JAM_RECOVERY_COMMAND);
        
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
    context<PrinterStateMachine>().SendMotorCommand(APPROACH_AFTER_JAM_COMMAND);

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

Pressing::Pressing(my_context ctx) : my_base(ctx)
{
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PressingState, Entering, uiSubState);
    
    // check to see if the door is still open after calibrating
    if (PRINTENGINE->DoorIsOpen())
        post_event(EvDoorOpened());
    else if (context<PrinterStateMachine>()._motionCompleted) 
        post_event(EvMotionCompleted());
}

Pressing::~Pressing()
{
    PRINTENGINE->SendStatus(PressingState, Leaving);
}

sc::result Pressing::react(const EvMotionCompleted&)
{
    double delay = PRINTENGINE->GetTrayDeflectionPauseTimeSec();
    if (delay < 0.001)
    {
        // we can skip the delay state
        context<PrinterStateMachine>().SendMotorCommand(UNPRESS_COMMAND);
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
    context<PrinterStateMachine>().SendMotorCommand(UNPRESS_COMMAND);

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
    return transit<PreExposureDelay>();
}

PreExposureDelay::PreExposureDelay(my_context ctx) : my_base(ctx)
{  
    UISubState uiSubState = PRINTENGINE->PauseRequested() ? AboutToPause : 
                                                            NoUISubState;
    
    PRINTENGINE->SendStatus(PreExposureDelayState, Entering, uiSubState);

    // check to see if the door is still open after calibrating,
    // in case we skipped the tray deflection steps
    if (PRINTENGINE->DoorIsOpen())
        post_event(EvDoorOpened());
    else
    {
        double delay = PRINTENGINE->GetPreExposureDelayTimeSec();
        if (delay < 0.001)
        {
            // no delay needed
            post_event(EvDelayEnded());
        }
        else
            PRINTENGINE->StartDelayTimer(delay);
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
int Exposing::_previousLayer = 0;

Exposing::Exposing(my_context ctx) : my_base(ctx)
{    
    // calculate time estimate before sending status
    double exposureTimeSec;
    if (_remainingExposureTimeSec > 0)
    {
        // we must be returning here after door open or pause
        exposureTimeSec = _remainingExposureTimeSec;
        int layer = _previousLayer;
        PRINTENGINE->SetCurrentLayer(layer);
        
        PRINTENGINE->SetEstimatedPrintTime(true);
        // adjust the estimated remaining print time 
        // by the remaining exposure time
        PRINTENGINE->DecreaseEstimatedPrintTime(
                PRINTENGINE->GetExposureTimeSec() - _remainingExposureTimeSec);  
    }
    else
    { 
        if (!PRINTENGINE->NextLayer())
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
    if (_remainingExposureTimeSec > 0.0)
    {
        _previousLayer = PRINTENGINE->GetCurrentLayerNum();
    }
    PRINTENGINE->SendStatus(ExposingState, Leaving);
}

sc::result Exposing::react(const EvExposed&)
{
    PRINTENGINE->ClearRotationInterrupt();
    
    // send the appropriate separation command to the motor controller, and
    // record the motor controller event we're waiting for
    context<PrinterStateMachine>().SendMotorCommand(SEPARATE_COMMAND);

    return transit<Separating>();
}

// Clear the information saved when leaving Exposing before the exposure is 
// actually completed
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
        LOGGER.LogMessage(LOG_INFO, msg);
        
        context<PrinterStateMachine>()._remainingUnjamTries = 
                                            SETTINGS.GetInt(MAX_UNJAM_TRIES);
        
        if (context<PrinterStateMachine>()._remainingUnjamTries > 0)
        {
            context<PrinterStateMachine>().SendMotorCommand(
                                                        JAM_RECOVERY_COMMAND);
            return transit<Unjamming>();
        }
        else
            return transit<Jammed>(); 
    }
    else
    {
        context<PrinterStateMachine>().SendMotorCommand(APPROACH_COMMAND);
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
    if (PRINTENGINE->NoMoreLayers())
    {
        PRINTENGINE->ClearCurrentPrint();
        context<PrinterStateMachine>()._homingSubState = PrintCompleted;
        context<PrinterStateMachine>().SendHomeCommand();
        
        if (IsInternetConnected())
            return transit<GettingFeedback>(); 
        else
            return transit<Homing>();    
    }
    else if (PRINTENGINE->PauseRequested())
    {    
        PRINTENGINE->SetInspectionRequested(false);
        context<PrinterStateMachine>().SendMotorCommand(
                                                    PAUSE_AND_INSPECT_COMMAND);
        return transit<MovingToPause>();
    }
    else
    {
        if (context<PrinterStateMachine>().HandlePressCommand())
            return transit<Pressing>(); 
        else
            return transit<PreExposureDelay>();
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


DemoMode::DemoMode(my_context ctx) : my_base(ctx)
{
    PRINTENGINE->SendStatus(DemoModeState, Entering);
    
    PRINTENGINE->SetDemoMode();
}

DemoMode::~DemoMode()
{
    PRINTENGINE->SendStatus(DemoModeState, Leaving);
}
