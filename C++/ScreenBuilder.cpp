//  File:   ScreenBuilder.cpp
//  Builds the screens shown on the front panel
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

#include <ScreenBuilder.h>
#include <PrinterStatus.h>
#include <ScreenLayouts.h>

// Build the screens and add them to a map keyed by PrintEngine states and UI
// sub states.
void ScreenBuilder::BuildScreens(std::map<PrinterStatusKey, Screen*>& screenMap) 
{
    ScreenText* unknown = new ScreenText();
    unknown->Add(new ScreenLine(UNKNOWN_SCREEN_LINE1));
    unknown->Add(new ReplaceableLine(UNKNOWN_SCREEN_LINE2));
    unknown->Add(new ReplaceableLine(UNKNOWN_SCREEN_LINE3));
    screenMap[UNKNOWN_SCREEN_KEY] =  
            new UnknownScreen(unknown, UNKNOWN_SCREEN_LED_SEQ);
    
    // NULL screens for states that shouldn't change what's already displayed
    screenMap[Key(PrinterOnState, NoUISubState)] = NULL;  
    screenMap[Key(InitializingState, NoUISubState)] = NULL;
    screenMap[Key(HomeState, NoUISubState)] = NULL;
    screenMap[Key(DoorClosedState, NoUISubState)] = NULL;
    screenMap[Key(ApproachingState, NoUISubState)] = NULL; 
    screenMap[Key(SeparatingState, NoUISubState)] = NULL; 
    screenMap[Key(PreExposureDelayState, NoUISubState)] = NULL; 
    screenMap[Key(ExposingState, NoUISubState)] = NULL; 
    screenMap[Key(PressingState, NoUISubState)] = NULL; 
    screenMap[Key(PressDelayState, NoUISubState)] = NULL; 
    screenMap[Key(UnpressingState, NoUISubState)] = NULL; 
    screenMap[Key(AwaitingCancelationState, NoUISubState)] = NULL; 
    
    ScreenText* readyLoaded = new ScreenText;
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE3));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE4));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE2));
    screenMap[Key(HomeState, HavePrintData)] = 
            new Screen(readyLoaded, READY_LOADED_LED_SEQ);
    
    ScreenText* startLoaded = new ScreenText;
    startLoaded->Add(new ReplaceableLine(START_LOADED_LINE1)); 
    startLoaded->Add(new ScreenLine(START_LOADED_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE3));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE4));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE5));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN1_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN2_LINE2));
    screenMap[Key(HomeState, LoadedPrintData)] = 
            new NamesScreen(startLoaded, START_LOADED_LED_SEQ);
    
    ScreenText* loadFail = new ScreenText;
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE1));
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE2));
    loadFail->Add(new ScreenLine(LOAD_FAIL_BTN2_LINE2));
    screenMap[Key(HomeState, PrintDataLoadFailed)] = 
            new Screen(loadFail, LOAD_FAIL_LED_SEQ);
    
    ScreenText* downloadFail = new ScreenText;
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_LINE1));
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_LINE2));
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_BTN2_LINE2));
    screenMap[Key(HomeState, PrintDownloadFailed)] = 
            new Screen(downloadFail, DOWNLOAD_FAIL_LED_SEQ);
    
    ScreenText* loadFail2 = new ScreenText;
    loadFail2->Add(new ScreenLine(LOAD_FAIL_LINE1));
    loadFail2->Add(new ScreenLine(LOAD_FAIL_LINE2));
    loadFail2->Add(new ScreenLine(LOAD_FAIL_BTN2_LINE2));
    screenMap[Key(DoorOpenState, PrintDataLoadFailed)] = 
            new Screen(loadFail2, LOAD_FAIL_LED_SEQ);
    
    ScreenText* downloadFail2 = new ScreenText;
    downloadFail2->Add(new ScreenLine(DOWNLOAD_FAIL_LINE1));
    downloadFail2->Add(new ScreenLine(DOWNLOAD_FAIL_LINE2));
    downloadFail2->Add(new ScreenLine(DOWNLOAD_FAIL_BTN2_LINE2));
    screenMap[Key(DoorOpenState, PrintDownloadFailed)] = 
            new Screen(downloadFail2, DOWNLOAD_FAIL_LED_SEQ);
    
    // the next screen contains the static portions of print status
    ScreenText* printing = new ScreenText;
    printing->Add(new ScreenLine(PRINTING_LINE1));
    printing->Add(new ReplaceableLine(PRINTING_LINE2));
    printing->Add(new ScreenLine(PRINTING_LINE4));
    printing->Add(new ReplaceableLine(PRINTING_LINE5));
    printing->Add(new ScreenLine(PRINTING_BTN1_LINE2));
    printing->Add(new ScreenLine(PRINTING_BTN2_LINE2));
    screenMap[Key(PrintingLayerState, NoUISubState)] = 
            new NamesScreen(printing, NO_LED_SEQ, false);
    
    // the next screen adds the remaining print time to print status
    ScreenText* countdown = new ScreenText;
    // clear the previously shown time
    countdown->Add(new ReplaceableLine(PRINTING_CLEAR_LINE3));
    // show the new remaining print time
    countdown->Add(new ReplaceableLine(PRINTING_LINE3));
    screenMap[Key(InitializingLayerState, NoUISubState)] = 
            new PrintStatusScreen(countdown, NO_LED_SEQ);  
    
    ScreenText* aboutToPause0 = new ScreenText;
    aboutToPause0->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(PreExposureDelayState, AboutToPause)] = 
            new Screen(aboutToPause0, ABOUT_TO_PAUSE_LED_SEQ, true, false); 
     
    ScreenText* aboutToPause1 = new ScreenText;
    aboutToPause1->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(ExposingState, AboutToPause)] = 
            new Screen(aboutToPause1, ABOUT_TO_PAUSE_LED_SEQ, true, false); 
     
    ScreenText* aboutToPause2 = new ScreenText;
    aboutToPause2->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(SeparatingState, AboutToPause)] = 
            new Screen(aboutToPause2, ABOUT_TO_PAUSE_LED_SEQ, true, false); 
    
    ScreenText* aboutToPause3 = new ScreenText;
    aboutToPause3->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(ApproachingState, AboutToPause)] = 
            new Screen(aboutToPause3, ABOUT_TO_PAUSE_LED_SEQ, true, false); 
   
    ScreenText* aboutToPause4 = new ScreenText;
    aboutToPause4->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(MovingToPauseState, NoUISubState)] = 
            new Screen(aboutToPause4, ABOUT_TO_PAUSE_LED_SEQ, true, false);

    ScreenText* aboutToPause5 = new ScreenText;
    aboutToPause5->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(PressingState, AboutToPause)] = 
            new Screen(aboutToPause5, ABOUT_TO_PAUSE_LED_SEQ, true, false);
    
    ScreenText* aboutToPause6 = new ScreenText;
    aboutToPause6->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(PressDelayState, AboutToPause)] = 
            new Screen(aboutToPause6, ABOUT_TO_PAUSE_LED_SEQ, true, false);
    
    ScreenText* aboutToPause7 = new ScreenText;
    aboutToPause7->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(UnpressingState, AboutToPause)] = 
            new Screen(aboutToPause7, ABOUT_TO_PAUSE_LED_SEQ, true, false);

    ScreenText* aboutToPause8 = new ScreenText;
    aboutToPause8->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[Key(InitializingLayerState, AboutToPause)] = 
            new Screen(aboutToPause8, ABOUT_TO_PAUSE_LED_SEQ, true, false);

    ScreenText* aboutToResume = new ScreenText;
    aboutToResume->Add(new ScreenLine(ABOUT_TO_RESUME_LINE1));
    screenMap[Key(MovingToResumeState, NoUISubState)] = 
            new Screen(aboutToResume, ABOUT_TO_RESUME_LED_SEQ, true, false);     
    
    ScreenText* paused = new ScreenText;
    paused->Add(new ScreenLine(PAUSED_LINE1));
    paused->Add(new ScreenLine(PAUSED_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN1_LINE1));
    paused->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN2_LINE2));
    // don't clear LEDs before showing paused screen's animation,
    // so that it will only animate those LEDs
    screenMap[Key(PausedState, NoUISubState)] = 
            new Screen(paused, PAUSED_LED_SEQ, true, false);
    
    ScreenText* unjamming = new ScreenText;
    unjamming->Add(new ScreenLine(UNJAMMING_LINE1));
    unjamming->Add(new ScreenLine(UNJAMMING_LINE2));
    unjamming->Add(new ScreenLine(UNJAMMING_LINE3));
    unjamming->Add(new ScreenLine(PAUSED_BTN1_LINE1));    
    unjamming->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    screenMap[Key(UnjammingState, NoUISubState)] = 
            new Screen(unjamming, UNJAMMING_LED_SEQ);
    
    ScreenText* jammed = new ScreenText;
    jammed->Add(new ScreenLine(JAMMED_LINE1));
    jammed->Add(new ScreenLine(JAMMED_LINE2));
    jammed->Add(new ScreenLine(JAMMED_LINE3));
    jammed->Add(new ScreenLine(PAUSED_BTN1_LINE1));    
    jammed->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    jammed->Add(new ScreenLine(PAUSED_BTN2_LINE2));
    screenMap[Key(JammedState, NoUISubState)] = 
            new Screen(jammed, JAMMED_LED_SEQ);
   
    ScreenText* cancelPrompt = new ScreenText;
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_LINE2));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN1_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN2_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN1_LINE2));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN2_LINE2));
    screenMap[Key(ConfirmCancelState, NoUISubState)] = 
            new Screen(cancelPrompt, CONFIRM_CANCEL_LED_SEQ);

    ScreenText* printComplete = new ScreenText;
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE1));
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE2));
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE3));
    screenMap[Key(HomingState, PrintCompleted)] = 
            new Screen(printComplete, PRINT_COMPLETE_LED_SEQ, true, false);    

    ScreenText* getFeedback = new ScreenText;
    getFeedback->Add(new ReplaceableLine(GET_FEEDBACK_LINE1));
    getFeedback->Add(new ScreenLine(GET_FEEDBACK_LINE2));
    getFeedback->Add(new ScreenLine(GET_FEEDBACK_LINE3));
    getFeedback->Add(new ScreenLine(GET_FEEDBACK_BTN1_LINE2));
    getFeedback->Add(new ScreenLine(GET_FEEDBACK_BTN2_LINE2));
    screenMap[Key(GettingFeedbackState, NoUISubState)] = 
            new NamesScreen(getFeedback, GET_FEEDBACK_LED_SEQ, false);   
    
    ScreenText* startingPrint = new ScreenText;
    startingPrint->Add(new ScreenLine(STARTING_PRINT_LINE1));
    startingPrint->Add(new ReplaceableLine(STARTING_PRINT_LINE2));
    startingPrint->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE1));
    startingPrint->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE2));
    screenMap[Key(MovingToStartPositionState, NoUISubState)] = 
            new NamesScreen(startingPrint, STARTING_PRINT_LED_SEQ);
        
    ScreenText* calibratePrompt = new ScreenText;
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_LINE1));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_LINE2));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_LINE3));
    calibratePrompt->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE1));
    calibratePrompt->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE2));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_BTN2_LINE1));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_BTN2_LINE2));
    screenMap[Key(MovingToStartPositionState, CalibratePrompt)] = 
            new NamesScreen(calibratePrompt, CALIBRATE_PROMPT_LED_SEQ);
    
    ScreenText* loadFirst = new ScreenText;
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE1));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE2));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE3));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE4));
    screenMap[Key(HomeState, NoPrintData)] = 
            new Screen(loadFirst, LOAD_FIRST_LED_SEQ);        
    
    ScreenText* downLoading = new ScreenText;
    downLoading->Add(new ScreenLine(DOWNLOADING_FILE_LINE1));
    screenMap[Key(HomeState, DownloadingPrintData)] = 
            new Screen(downLoading, DOWNLOADING_FILE_LED_SEQ); 
    
    ScreenText* loading = new ScreenText;
    loading->Add(new ScreenLine(LOADING_FILE_LINE1));
    screenMap[Key(HomeState, LoadingPrintData)] = 
            new Screen(loading, LOADING_FILE_LED_SEQ);        
    
    ScreenText* downLoading2 = new ScreenText;
    downLoading2->Add(new ScreenLine(DOWNLOADING_FILE_LINE1));
    screenMap[Key(DoorOpenState, DownloadingPrintData)] = 
            new Screen(downLoading2, DOWNLOADING_FILE_LED_SEQ); 
    
    ScreenText* loading2 = new ScreenText;
    loading2->Add(new ScreenLine(LOADING_FILE_LINE1));
    screenMap[Key(DoorOpenState, LoadingPrintData)] = 
            new Screen(loading2, LOADING_FILE_LED_SEQ);        
    
    ScreenText* printCanceled = new ScreenText;
    printCanceled->Add(new ScreenLine(CANCELED_LINE1));
    printCanceled->Add(new ScreenLine(CANCELED_LINE2));
    printCanceled->Add(new ScreenLine(CANCELED_LINE3));
    screenMap[Key(HomingState, PrintCanceled)] = 
            new Screen(printCanceled, CANCELED_LED_SEQ);    
    
    ScreenText* doorOpen = new ScreenText;
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE1));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE2));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE3));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE4));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE5));
    screenMap[Key(DoorOpenState, NoUISubState)] = 
            new Screen(doorOpen, DOOR_OPEN_LED_SEQ); 
    
    ScreenText* loadedDdoorOpen = new ScreenText;
    loadedDdoorOpen->Add(new ReplaceableLine(LOADED_DOOR_OPEN_LINE1));
    loadedDdoorOpen->Add(new ScreenLine(LOADED_DOOR_OPEN_LINE2));
    loadedDdoorOpen->Add(new ScreenLine(LOADED_DOOR_OPEN_LINE3));
    screenMap[Key(DoorOpenState, LoadedPrintData)] = 
            new NamesScreen(loadedDdoorOpen, LOADED_DOOR_OPEN_LED_SEQ); 
    
    // when leaving door opened, just clear the screen,
    // in case next state has no screen defined
    ScreenText* doorClosed = new ScreenText;
    screenMap[Key(DoorOpenState, ClearingScreen)] = new Screen(doorClosed, 0);
    
    ScreenText* error = new ScreenText;
    error->Add(new ScreenLine(ERROR_CODE_LINE1));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE2));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE3));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE4));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE5));
    error->Add(new ScreenLine(ERROR_CODE_BTN1_LINE2));
    screenMap[Key(ErrorState, NoUISubState)] = 
            new ErrorScreen(error, ERROR_CODE_LED_SEQ);   
    
    ScreenText* homing = new ScreenText;
    homing->Add(new ScreenLine(HOMING_LINE1));
    homing->Add(new ScreenLine(HOMING_LINE2));
    screenMap[Key(HomingState, NoUISubState)] = 
            new Screen(homing, HOMING_LED_SEQ);
    
    ScreenText* version = new ScreenText;
    version->Add(new ScreenLine(SYSINFO_LINE1));
    version->Add(new ScreenLine(SYSINFO_LINE2));
    version->Add(new ScreenLine(SYSINFO_LINE3));
    version->Add(new ReplaceableLine(SYSINFO_LINE4));
    version->Add(new ReplaceableLine(SYSINFO_BTN1_LINE1));
    version->Add(new ReplaceableLine(SYSINFO_BTN1_LINE2));
    version->Add(new ScreenLine(SYSINFO_BTN2_LINE2));
    screenMap[Key(ShowingVersionState, NoUISubState)] = 
            new SysInfoScreen(version, SYSINFO_LED_SEQ);           
    
    ScreenText* calibrating = new ScreenText;
    calibrating->Add(new ScreenLine(CALIBRATING_LINE1));
    calibrating->Add(new ScreenLine(CALIBRATING_LINE2));
    calibrating->Add(new ScreenLine(CALIBRATING_BTN2_LINE2));
    calibrating->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE1));
    calibrating->Add(new ScreenLine(CANCEL_PRINT_BTN1_LINE2));
    screenMap[Key(CalibratingState, NoUISubState)] = 
            new Screen(calibrating, CALIBRATING_LED_SEQ);  
    

    ScreenText* pair = new ScreenText;
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE1));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE2));
    pair->Add(new ReplaceableLine(PAIR_ACCOUNT_LINE3));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE4));
    pair->Add(new ReplaceableLine(PAIR_ACCOUNT_LINE5));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_BTN1_LINE2));
    screenMap[Key(RegisteringState, NoUISubState)] = 
            new RegistrationScreen(pair, PAIR_ACCOUNT_LED_SEQ);       

    ScreenText* pairSuccess = new ScreenText;
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE1));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE2));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE3));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE4));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE5));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_BTN2_LINE2));
    screenMap[Key(HomeState, Registered)] = 
            new Screen(pairSuccess, PAIR_SUCCESS_LED_SEQ);   
    
    ScreenText* wifiConnecting = new ScreenText;
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_LINE1));
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_LINE2));
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_BTN2_LINE2));
    screenMap[Key(HomeState, WiFiConnecting)] = 
            new Screen(wifiConnecting, WIFI_CONNECTING_LED_SEQ);    
    
    ScreenText* wifiNotConnected = new ScreenText;
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_LINE1));
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_LINE2));
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_BTN2_LINE2));
    screenMap[Key(HomeState, WiFiConnectionFailed)] = 
            new Screen(wifiNotConnected, WIFI_NOT_CONNECTED_LED_SEQ);    
    
    ScreenText* wifiConnected = new ScreenText;
    wifiConnected->Add(new ScreenLine(WIFI_CONNECTED_LINE1));
    wifiConnected->Add(new ScreenLine(WIFI_CONNECTED_BTN2_LINE2));
    screenMap[Key(HomeState, WiFiConnected)] = 
            new Screen(wifiConnected, WIFI_CONNECTED_LED_SEQ);

    ScreenText* demoMode = new ScreenText;
    demoMode->Add(new ScreenLine(DEMO_SCREEN_LINE1));
    demoMode->Add(new ScreenLine(DEMO_SCREEN_LINE2));
    screenMap[Key(DemoModeState, NoUISubState)] = 
            new Screen(demoMode, DEMO_SCREEN_LED_SEQ, false, false);
    
    ScreenText* usbError = new ScreenText;
    usbError->Add(new ScreenLine(USB_FILE_FOUND_LINE1));
    usbError->Add(new ReplaceableLine(USB_FILE_FOUND_LINE2));
    usbError->Add(new ReplaceableLine(USB_FILE_FOUND_LINE3));
    usbError->Add(new ReplaceableLine(USB_FILE_FOUND_LINE4));
    usbError->Add(new ScreenLine(USB_FILE_FOUND_BTN1_LINE2));
    usbError->Add(new ScreenLine(USB_FILE_FOUND_BTN2_LINE2));
    screenMap[Key(HomeState, USBDriveFileFound)] = 
            new USBFileFoundScreen(usbError, USB_FILE_FOUND_LED_SEQ);

    ScreenText* usbFileFound = new ScreenText;
    usbFileFound->Add(new ScreenLine(USB_DRIVE_ERROR_LINE1));
    usbFileFound->Add(new ScreenLine(USB_DRIVE_ERROR_LINE2));
    usbFileFound->Add(new ScreenLine(USB_DRIVE_ERROR_LINE3));
    usbFileFound->Add(new ReplaceableLine(USB_DRIVE_ERROR_LINE4));
    usbFileFound->Add(new ScreenLine(USB_DRIVE_ERROR_LINE5));
    usbFileFound->Add(new ScreenLine(USB_DRIVE_ERROR_BTN2_LINE2));
    screenMap[Key(HomeState, USBDriveError)] = 
            new USBErrorScreen(usbFileFound, USB_DRIVE_ERROR_LED_SEQ); 
    
    ScreenText* confirmUpgrade = new ScreenText;
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_LINE1));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_LINE2));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_LINE3));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_LINE4));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_BTN1_LINE1));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_BTN1_LINE2));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_BTN2_LINE1));
    confirmUpgrade->Add(new ScreenLine(CONFIRM_UPGRADE_BTN2_LINE2));
    screenMap[Key(ConfirmUpgradeState, NoUISubState)] = 
            new Screen(confirmUpgrade, CONFIRM_UPGRADE_LED_SEQ); 
    
    // clear screen when leaving confirmUpgrade, so that upgrading screen
    // doesn't have to clear & redraw when it only wants to update the LED ring
    ScreenText* leavingConfirmUpgrade = new ScreenText;
    screenMap[Key(ConfirmUpgradeState, ClearingScreen)] = 
                                        new Screen(leavingConfirmUpgrade, 0);

    ScreenText* upgradingProjector = new ScreenText;
    upgradingProjector->Add(new ScreenLine(UPGRADING_PROJECTOR_LINE1));
    upgradingProjector->Add(new ScreenLine(UPGRADING_PROJECTOR_LINE2));
    upgradingProjector->Add(new ScreenLine(UPGRADING_PROJECTOR_LINE3));
    upgradingProjector->Add(new ScreenLine(UPGRADING_PROJECTOR_LINE4));
    screenMap[Key(UpgradingProjectorState, NoUISubState)] = 
         new ProjectorUpgradingScreen(upgradingProjector, NO_LED_SEQ); 

    ScreenText* projectorUpgraded = new ScreenText;
    projectorUpgraded->Add(new ScreenLine(PROJECTOR_UPGRADED_LINE1));
    projectorUpgraded->Add(new ScreenLine(PROJECTOR_UPGRADED_LINE2));
    projectorUpgraded->Add(new ScreenLine(PROJECTOR_UPGRADED_LINE3));
    projectorUpgraded->Add(new ScreenLine(PROJECTOR_UPGRADED_LINE4));
    screenMap[Key(UpgradeCompleteState, NoUISubState)] = 
            new Screen(projectorUpgraded, PROJECTOR_UPGRADED_LED_SEQ); 
}

