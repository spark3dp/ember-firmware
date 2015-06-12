/* 
 * File:   ScreenBuilder.cpp
 * Author: Richard Greene
 * 
 * Builds the screens shown on the front panel.
 * 
 * Created on July 23, 2014, 12:15 PM
 */

#include <ScreenBuilder.h>
#include <PrinterStatus.h>
#include <ScreenLayouts.h>

/// Build the screens and add them to a map keyed by PrintEngine states and UI
/// sub states.
void ScreenBuilder::BuildScreens(std::map<PrinterStatusKey, Screen*>& screenMap) 
{
    ScreenText* unknown = new ScreenText();
    unknown->Add(new ScreenLine(UNKNOWN_SCREEN_LINE1));
    unknown->Add(new ReplaceableLine(UNKNOWN_SCREEN_LINE2));
    unknown->Add(new ReplaceableLine(UNKNOWN_SCREEN_LINE3));
    screenMap[UNKNOWN_SCREEN_KEY] =  new UnknownScreen(unknown, 
                                                       UNKNOWN_SCREEN_LED_SEQ);
    
    // NULL screens for states that shouldn't change what's already displayed
    screenMap[PS_KEY(PrinterOnState, NoUISubState)] = NULL;  
    screenMap[PS_KEY(InitializingState, NoUISubState)] = NULL;
    screenMap[PS_KEY(HomeState, NoUISubState)] = NULL;
    screenMap[PS_KEY(DoorClosedState, NoUISubState)] = NULL;
    screenMap[PS_KEY(ApproachingState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(ExposingState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(PreExposureDelayState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(PressingState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(PressDelayState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(UnpressingState, NoUISubState)] = NULL; 
    screenMap[PS_KEY(AwaitingCancelationState, NoUISubState)] = NULL; 
    
    ScreenText* readyLoaded = new ScreenText;
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE3));
    readyLoaded->Add(new ScreenLine(READY_LOADED_LINE4));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN1_LINE2));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE1));
    readyLoaded->Add(new ScreenLine(READY_LOADED_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, HavePrintData)] = 
                                new Screen(readyLoaded, READY_LOADED_LED_SEQ);
    
    ScreenText* startLoaded = new ScreenText;
    startLoaded->Add(new ReplaceableLine(START_LOADED_LINE1)); 
    startLoaded->Add(new ScreenLine(START_LOADED_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE3));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE4));
    startLoaded->Add(new ScreenLine(START_LOADED_LINE5));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN1_LINE2));
    startLoaded->Add(new ScreenLine(START_LOADED_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, LoadedPrintData)] = 
                        new JobNameScreen(startLoaded, START_LOADED_LED_SEQ);
    
    ScreenText* loadFail = new ScreenText;
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE1));
    loadFail->Add(new ScreenLine(LOAD_FAIL_LINE2));
    loadFail->Add(new ScreenLine(LOAD_FAIL_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, PrintDataLoadFailed)] = 
                                new Screen(loadFail, LOAD_FAIL_LED_SEQ);
    
    ScreenText* downloadFail = new ScreenText;
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_LINE1));
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_LINE2));
    downloadFail->Add(new ScreenLine(DOWNLOAD_FAIL_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, PrintDownloadFailed)] = 
                                new Screen(downloadFail, DOWNLOAD_FAIL_LED_SEQ);
    
    // the next screen contains the static portions of print status
    ScreenText* printing = new ScreenText;
    printing->Add(new ScreenLine(PRINTING_LINE1));
    printing->Add(new ReplaceableLine(PRINTING_LINE2));
    printing->Add(new ScreenLine(PRINTING_LINE4));
    printing->Add(new ScreenLine(PRINTING_BTN1_LINE2));
    printing->Add(new ScreenLine(PRINTING_BTN2_LINE2));
    screenMap[PS_KEY(PrintingLayerState, NoUISubState)] = 
                             new JobNameScreen(printing, PRINTING_LED_SEQ);
    
    // the next screen adds the remaining print time to print status
    ScreenText* countdown = new ScreenText;
    // clear the previously shown time
    countdown->Add(new ReplaceableLine(PRINTING_CLEAR_LINE3));
    // show the new remaining print time
    countdown->Add(new ReplaceableLine(PRINTING_LINE3));
    screenMap[PS_KEY(SeparatingState, NoUISubState)] = 
                             new PrintStatusScreen(countdown, PRINTING_LED_SEQ);  
    
    ScreenText* aboutToPause0 = new ScreenText;
    aboutToPause0->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(PreExposureDelayState, AboutToPause)] = 
                            new Screen(aboutToPause0, ABOUT_TO_PAUSE_LED_SEQ,
                                                                   true, false); 
     
    ScreenText* aboutToPause1 = new ScreenText;
    aboutToPause1->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(ExposingState, AboutToPause)] = 
                            new Screen(aboutToPause1, ABOUT_TO_PAUSE_LED_SEQ,
                                                                   true, false); 
     
    ScreenText* aboutToPause2 = new ScreenText;
    aboutToPause2->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(SeparatingState, AboutToPause)] = 
                            new Screen(aboutToPause2, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                   true, false); 
    ScreenText* aboutToPause3 = new ScreenText;
    aboutToPause3->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(ApproachingState, AboutToPause)] = 
                            new Screen(aboutToPause3, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                   true, false); 
   
    ScreenText* aboutToPause4 = new ScreenText;
    aboutToPause4->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(MovingToPauseState, NoUISubState)] = 
                            new Screen(aboutToPause4, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                  true, false);

    ScreenText* aboutToPause5 = new ScreenText;
    aboutToPause5->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(PressingState, AboutToPause)] = 
                            new Screen(aboutToPause5, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                  true, false);
    ScreenText* aboutToPause6 = new ScreenText;
    aboutToPause6->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(PressDelayState, AboutToPause)] = 
                            new Screen(aboutToPause6, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                  true, false);
    ScreenText* aboutToPause7 = new ScreenText;
    aboutToPause7->Add(new ScreenLine(ABOUT_TO_PAUSE_LINE1));
    screenMap[PS_KEY(UnpressingState, AboutToPause)] = 
                            new Screen(aboutToPause7, ABOUT_TO_PAUSE_LED_SEQ, 
                                                                  true, false);

    ScreenText* aboutToResume = new ScreenText;
    aboutToResume->Add(new ScreenLine(ABOUT_TO_RESUME_LINE1));
    screenMap[PS_KEY(MovingToResumeState, NoUISubState)] = 
                            new Screen(aboutToResume, ABOUT_TO_RESUME_LED_SEQ,
                                                                   true, false);     
    
    ScreenText* paused = new ScreenText;
    paused->Add(new ScreenLine(PAUSED_LINE1));
    paused->Add(new ScreenLine(PAUSED_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN1_LINE1));
    paused->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    paused->Add(new ScreenLine(PAUSED_BTN2_LINE2));
    // don't clear LEDs before showing paused screen's animation,
    // so that it will only animate those LEDs
    screenMap[PS_KEY(PausedState, NoUISubState)] = 
                                new Screen(paused, PAUSED_LED_SEQ, true, false);
    
    ScreenText* unjamming = new ScreenText;
    unjamming->Add(new ScreenLine(UNJAMMING_LINE1));
    unjamming->Add(new ScreenLine(UNJAMMING_LINE2));
    unjamming->Add(new ScreenLine(UNJAMMING_LINE3));
    unjamming->Add(new ScreenLine(PAUSED_BTN1_LINE1));    
    unjamming->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    screenMap[PS_KEY(UnjammingState, NoUISubState)] = 
                                new Screen(unjamming, UNJAMMING_LED_SEQ);
    
    ScreenText* jammed = new ScreenText;
    jammed->Add(new ScreenLine(JAMMED_LINE1));
    jammed->Add(new ScreenLine(JAMMED_LINE2));
    jammed->Add(new ScreenLine(JAMMED_LINE3));
    jammed->Add(new ScreenLine(PAUSED_BTN1_LINE1));    
    jammed->Add(new ScreenLine(PAUSED_BTN1_LINE2));
    jammed->Add(new ScreenLine(PAUSED_BTN2_LINE2));
    screenMap[PS_KEY(JammedState, NoUISubState)] = 
                                new Screen(jammed, JAMMED_LED_SEQ);
   
    ScreenText* cancelPrompt = new ScreenText;
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_LINE2));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN1_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN2_LINE1));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN1_LINE2));
    cancelPrompt->Add(new ScreenLine(CONFIRM_CANCEL_BTN2_LINE2));
    screenMap[PS_KEY(ConfirmCancelState, NoUISubState)] = 
                            new Screen(cancelPrompt, CONFIRM_CANCEL_LED_SEQ);

    ScreenText* printComplete = new ScreenText;
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE1));
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE2));
    printComplete->Add(new ScreenLine(PRINT_COMPLETE_LINE3));
    screenMap[PS_KEY(HomingState, PrintCompleted)] = 
                            new Screen(printComplete, PRINT_COMPLETE_LED_SEQ, 
                                                                  true, false);    
            
    ScreenText* startingPrint = new ScreenText;
    startingPrint->Add(new ScreenLine(STARTING_PRINT_LINE1));
    startingPrint->Add(new ReplaceableLine(STARTING_PRINT_LINE2));
    screenMap[PS_KEY(MovingToStartPositionState, NoUISubState)] = 
                       new JobNameScreen(startingPrint, STARTING_PRINT_LED_SEQ);
        
    ScreenText* calibratePrompt = new ScreenText;
    calibratePrompt->Add(new ScreenLine(STARTING_PRINT_LINE1));
    calibratePrompt->Add(new ReplaceableLine(STARTING_PRINT_LINE2));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_LINE1));
    calibratePrompt->Add(new ScreenLine(CALIBRATE_PROMPT_BTN2_LINE2));
    screenMap[PS_KEY(MovingToStartPositionState, CalibratePrompt)] = 
                       new JobNameScreen(calibratePrompt, CALIBRATE_PROMPT_LED_SEQ);
    
    ScreenText* loadFirst = new ScreenText;
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE1));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE2));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE3));
    loadFirst->Add(new ScreenLine(LOAD_FIRST_LINE4));
    screenMap[PS_KEY(HomeState, NoPrintData)] = 
                            new Screen(loadFirst, LOAD_FIRST_LED_SEQ);        
    
    ScreenText* downLoading = new ScreenText;
    downLoading->Add(new ScreenLine(DOWNLOADING_FILE_LINE1));
    screenMap[PS_KEY(HomeState, DownloadingPrintData)] = 
                            new Screen(downLoading, DOWNLOADING_FILE_LED_SEQ); 
    
    ScreenText* loading = new ScreenText;
    loading->Add(new ScreenLine(LOADING_FILE_LINE1));
    screenMap[PS_KEY(HomeState, LoadingPrintData)] = 
                            new Screen(loading, LOADING_FILE_LED_SEQ);        
    
    ScreenText* printCanceled = new ScreenText;
    printCanceled->Add(new ScreenLine(CANCELED_LINE1));
    printCanceled->Add(new ScreenLine(CANCELED_LINE2));
    printCanceled->Add(new ScreenLine(CANCELED_LINE3));
    screenMap[PS_KEY(HomingState, PrintCanceled)] = 
                            new Screen(printCanceled, CANCELED_LED_SEQ);    
    
    ScreenText* doorOpen = new ScreenText;
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE1));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE2));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE3));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE4));
    doorOpen->Add(new ScreenLine(DOOR_OPEN_LINE5));
    screenMap[PS_KEY(DoorOpenState, NoUISubState)] = 
                            new Screen(doorOpen, DOOR_OPEN_LED_SEQ); 
    
    // when leaving door opened, just clear the screen,
    // in case next state has no screen defined
    ScreenText* doorClosed = new ScreenText;
    screenMap[PS_KEY(DoorOpenState, ExitingDoorOpen)] = 
                            new Screen(doorClosed, 0);
    
    ScreenText* error = new ScreenText;
    error->Add(new ScreenLine(ERROR_CODE_LINE1));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE2));
    error->Add(new ReplaceableLine(ERROR_CODE_LINE3));
    error->Add(new ScreenLine(ERROR_CODE_BTN1_LINE2));
    screenMap[PS_KEY(ErrorState, NoUISubState)] = 
                            new ErrorScreen(error, ERROR_CODE_LED_SEQ);   
    
    ScreenText* homing = new ScreenText;
    homing->Add(new ScreenLine(HOMING_LINE1));
    homing->Add(new ScreenLine(HOMING_LINE2));
    screenMap[PS_KEY(HomingState, NoUISubState)] = 
                            new Screen(homing, HOMING_LED_SEQ);
    
    ScreenText* version = new ScreenText;
    version->Add(new ScreenLine(SYSINFO_LINE1));
    version->Add(new ScreenLine(SYSINFO_LINE2));
    version->Add(new ScreenLine(SYSINFO_LINE3));
    version->Add(new ReplaceableLine(SYSINFO_LINE4));
    version->Add(new ScreenLine(SYSINFO_BTN2_LINE2));
    screenMap[PS_KEY(ShowingVersionState, NoUISubState)] = 
                            new SysInfoScreen(version, SYSINFO_LED_SEQ);           
    
    ScreenText* calibrating = new ScreenText;
    calibrating->Add(new ScreenLine(CALIBRATING_LINE1));
    calibrating->Add(new ScreenLine(CALIBRATING_LINE2));
    calibrating->Add(new ScreenLine(CALIBRATING_BTN2_LINE2));
    screenMap[PS_KEY(CalibratingState, NoUISubState)] = 
                            new Screen(calibrating, CALIBRATING_LED_SEQ);  
    

    ScreenText* pair = new ScreenText;
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE1));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE2));
    pair->Add(new ReplaceableLine(PAIR_ACCOUNT_LINE3));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_LINE4));
    pair->Add(new ReplaceableLine(PAIR_ACCOUNT_LINE5));
    pair->Add(new ScreenLine(PAIR_ACCOUNT_BTN1_LINE2));
    screenMap[PS_KEY(RegisteringState, NoUISubState)] = 
                            new RegistrationScreen(pair, PAIR_ACCOUNT_LED_SEQ);       

    ScreenText* pairSuccess = new ScreenText;
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE1));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE2));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE3));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE4));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_LINE5));
    pairSuccess->Add(new ScreenLine(PAIR_SUCCESS_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, Registered)] = 
                            new Screen(pairSuccess, PAIR_SUCCESS_LED_SEQ);   
    
    ScreenText* wifiConnecting = new ScreenText;
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_LINE1));
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_LINE2));
    wifiConnecting->Add(new ScreenLine(WIFI_CONNECTING_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, WiFiConnecting)] = 
                            new Screen(wifiConnecting, WIFI_CONNECTING_LED_SEQ);    
    
    ScreenText* wifiNotConnected = new ScreenText;
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_LINE1));
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_LINE2));
    wifiNotConnected->Add(new ScreenLine(WIFI_NOT_CONNECTED_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, WiFiConnectionFailed)] = 
                       new Screen(wifiNotConnected, WIFI_NOT_CONNECTED_LED_SEQ);    
    
    ScreenText* wifiConnected = new ScreenText;
    wifiConnected->Add(new ScreenLine(WIFI_CONNECTED_LINE1));
    wifiConnected->Add(new ScreenLine(WIFI_CONNECTED_BTN2_LINE2));
    screenMap[PS_KEY(HomeState, WiFiConnected)] = 
                            new Screen(wifiConnected, WIFI_CONNECTED_LED_SEQ);    
    }

