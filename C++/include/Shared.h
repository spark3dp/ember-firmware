//  File:   Shared.h
//  String constants shared between different firmware & software components
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
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

#ifndef SHARED_H
#define	SHARED_H

constexpr const char* VERSION_MAJOR = "3";
constexpr const char* VERSION_MINOR = "2";

// named pipes
constexpr const char* COMMAND_PIPE           = "/tmp/CommandPipe";
constexpr const char* STATUS_TO_WEB_PIPE     = "/tmp/StatusToWebPipe";

constexpr const char* ROOT_DIR               = "/var/smith";

// path (relative to ROOT_DIR) to file containing all current smith settings
constexpr const char* SETTINGS_FILE          = "/config/settings";

// path to print settings file containing settings from web 
constexpr const char* TEMP_SETTINGS_FILE             = "/tmp/print_settings";

// path to file with registration values for display on front panel
// during primary registration
constexpr const char* PRIMARY_REGISTRATION_INFO_FILE = "/tmp/printer_registration";

// path to file with latest printer status
constexpr const char* PRINTER_STATUS_FILE            = "/run/printer_status";

// path to file written by smith-client, indicating Internet connection status
constexpr const char* SMITH_STATE_FILE               = "/var/local/smith_state";

// JSON key for Internet connection status
constexpr const char* INTERNET_CONNECTED_KEY         = "internet_connected";

// command strings
constexpr const char* CMD_START_PRINT                     = "START";
constexpr const char* CMD_CANCEL                          = "CANCEL";
constexpr const char* CMD_PAUSE                           = "PAUSE";
constexpr const char* CMD_RESUME                          = "RESUME";
constexpr const char* CMD_RESET_PRINTER                   = "RESET";
constexpr const char* CMD_REFRESH_SETTINGS                = "REFRESH";
constexpr const char* CMD_APPLY_SETTINGS                  = "APPLYSETTINGS";
constexpr const char* CMD_TEST                            = "TEST";
constexpr const char* CMD_CAL_IMAGE                       = "CALIMAGE";
constexpr const char* CMD_EXIT                            = "EXIT";
constexpr const char* CMD_QUIT                            = "QUIT";
constexpr const char* CMD_SHOW_PRINT_DATA_DOWNLOADING     = "SHOWPRINTDATADOWNLOADING";
constexpr const char* CMD_SHOW_PRINT_DOWNLOAD_FAILED      = "SHOWPRINTDOWNLOADFAILED";
constexpr const char* CMD_START_PRINT_DATA_LOAD           = "STARTPRINTDATALOAD";
constexpr const char* CMD_SHOW_PRINT_DATA_LOADED          = "SHOWPRINTDATALOADED";
constexpr const char* CMD_PROCESS_PRINT_DATA              = "PROCESSPRINTDATA";
constexpr const char* CMD_REGISTRATION_CODE               = "DISPLAYPRIMARYREGISTRATIONCODE";
constexpr const char* CMD_REGISTERED                      = "PRIMARYREGISTRATIONSUCCEEDED";
constexpr const char* CMD_SHOW_WIRELESS_CONNECTING        = "SHOWWIRELESSCONNECTING";
constexpr const char* CMD_SHOW_WIRELESS_CONNECTION_FAILED = "SHOWWIRELESSCONNECTIONFAILED";
constexpr const char* CMD_SHOW_WIRELESS_CONNECTED         = "SHOWWIRELESSCONNECTED";
constexpr const char* CMD_DISMISS                         = "DISMISS";
constexpr const char* CMD_BTN1                            = "BUTTON1";
constexpr const char* CMD_BTN1_HOLD                       = "BUTTON1HOLD";
constexpr const char* CMD_BTN2                            = "BUTTON2";
constexpr const char* CMD_BTN2_HOLD                       = "BUTTON2HOLD";
constexpr const char* CMD_BTNS_1_AND_2                    = "BUTTONS1AND2";
constexpr const char* CMD_BTNS_1_AND_2_HOLD               = "BUTTONS1AND2HOLD";
constexpr const char* CMD_SHOW_WHITE                      = "SHOWWHITE";
constexpr const char* CMD_SHOW_BLACK                      = "SHOWBLACK";

// JSON keys for PrinterStatus sent to web
constexpr const char* STATE_PS_KEY                  = "state";
constexpr const char* UISUBSTATE_PS_KEY             = "ui_sub_state";
constexpr const char* CHANGE_PS_KEY                 = "change";
constexpr const char* IS_ERROR_PS_KEY               = "is_error";
constexpr const char* ERROR_CODE_PS_KEY             = "error_code";
constexpr const char* ERRNO_PS_KEY                  = "errno";
constexpr const char* ERROR_MSG_PS_KEY              = "error_message";
constexpr const char* JOB_NAME_PS_KEY               = "job_name";
constexpr const char* JOB_ID_PS_KEY                 = "job_id";
constexpr const char* LAYER_PS_KEY                  = "layer";
constexpr const char* TOTAL_LAYERS_PS_KEY           = "total_layers";
constexpr const char* SECONDS_LEFT_PS_KEY           = "seconds_left";
constexpr const char* TEMPERATURE_PS_KEY            = "temperature";
constexpr const char* PRINT_RATING_PS_KEY           = "print_rating";
constexpr const char* SPARK_STATE_PS_KEY            = "spark_state";
constexpr const char* SPARK_JOB_STATE_PS_KEY        = "spark_job_state";
constexpr const char* LOCAL_JOB_UUID_PS_KEY         = "spark_local_job_uuid";
constexpr const char* CAN_LOAD_PS_KEY               = "can_load_print_data";
constexpr const char* CAN_UPGRADE_PROJECTOR_PS_KEY  = "can_upgrade_projector";

// StaeChange enum names
constexpr const char* NO_CHANGE               = "none";
constexpr const char* ENTERING                = "entering";
constexpr const char* LEAVING                 = "leaving";

// Print feedback enum names
constexpr const char* UNKNOWN_PRINT_FEEDBACK  = "unknown";
constexpr const char* PRINT_SUCCESSFUL        = "successful";
constexpr const char* PRINT_FAILED            = "failed";

// PrinterStateMachine state names
constexpr const char* PRINTER_ON_STATE               = "PrinterOn";
constexpr const char* DOOR_CLOSED_STATE              = "DoorClosed";
constexpr const char* INITIALIZING_STATE             = "Initializing";
constexpr const char* DOOR_OPEN_STATE                = "DoorOpen";
constexpr const char* HOMING_STATE                   = "Homing";
constexpr const char* HOME_STATE                     = "Home";
constexpr const char* ERROR_STATE                    = "Error";
constexpr const char* MOVING_TO_START_POSITION_STATE = "MovingToStartPosition";
constexpr const char* INITIALIZING_LAYER_STATE       = "InitializingLayer";
constexpr const char* PRESSING_STATE                 = "Pressing";
constexpr const char* PRESS_DELAY_STATE              = "PressDelay";
constexpr const char* UNPRESSING_STATE               = "Unpressing";
constexpr const char* PRE_EXPOSURE_DELAY_STATE       = "PreExposureDelay";
constexpr const char* EXPOSING_STATE                 = "Exposing";
constexpr const char* PRINTING_STATE                 = "Printing";
constexpr const char* PRINTING_LAYER_STATE           = "PrintingLayer";
constexpr const char* MOVING_TO_PAUSE_STATE          = "MovingToPause";
constexpr const char* PAUSED_STATE                   = "Paused";
constexpr const char* MOVING_TO_RESUME_STATE         = "MovingToResume";
constexpr const char* SEPARATING_STATE               = "Separating";
constexpr const char* APPROACHING_STATE              = "Approaching";
constexpr const char* GETING_FEEDBACK_STATE          = "GettingFeedback";
constexpr const char* CONFIRM_CANCEL_STATE           = "ConfirmCancel";
constexpr const char* AWAITING_CANCELATION_STATE     = "AwaitingCancelation";
constexpr const char* SHOWING_VERSION_STATE          = "ShowingVersion";
constexpr const char* CALIBRATING_STATE              = "Calibrating";
constexpr const char* REGISTERING_STATE              = "Registering";
constexpr const char* UNJAMMING_STATE                = "Unjamming";
constexpr const char* JAMMED_STATE                   = "Jammed";
constexpr const char* DEMO_MODE_STATE                = "DemoMode";
constexpr const char* CONFIRM_UPGRADE_STATE          = "ConfirmUpgrade";
constexpr const char* UPGRADING_PROJECTOR_STATE      = "UpgradingProjector";
constexpr const char* UPGRADE_COMPLETE_STATE         = "UpgradeComplete";

// PrinterStateMachine UI sub-state names
constexpr const char* NO_SUBSTATE                     = "NoUISubState";
constexpr const char* NO_PRINT_DATA_SUBSTATE          = "NoPrintData";
constexpr const char* DOWNLOADING_PRINT_DATA_SUBSTATE = "DownloadingPrintData";
constexpr const char* PRINT_DOWNLOAD_FAILED_SUBSTATE  = "PrintDownloadFailed";
constexpr const char* LOADING_PRINT_DATA_SUBSTATE     = "LoadingPrintData";
constexpr const char* LOADED_PRINT_DATA_SUBSTATE      = "LoadedPrintData";
constexpr const char* PRINT_DATA_LOAD_FAILED_SUBSTATE = "PrintDataLoadFailed";
constexpr const char* HAVE_PRINT_DATA_SUBSTATE        = "HavePrintData";
constexpr const char* PRINT_CANCELED_SUBSTATE         = "PrintCanceled";
constexpr const char* PRINT_COMPLETED_SUBSTATE        = "PrintCompleted";
constexpr const char* CLEARING_SCREEN_SUBSTATE        = "ClearingScreen";
constexpr const char* REGISTERED_SUBSTATE             = "Registered";
constexpr const char* ABOUT_TO_PAUSE_SUBSTATE         = "AboutToPause";
constexpr const char* WIFI_CONNECTING_SUBSTATE        = "WiFiConnecting";
constexpr const char* WIFI_CONNECTION_FAILED_SUBSTATE = "WiFiConnectionFailed";
constexpr const char* WIFI_CONNECTED_SUBSTATE         = "WiFiConnected";
constexpr const char* CALIBRATE_PROMPT_SUBSTATE       = "CalibratePrompt";
constexpr const char* USB_FILE_FOUND_SUBSTATE         = "USBDriveFileFound";
constexpr const char* USB_DRIVE_ERROR_SUBSTATE        = "USBDriveError";

// JSON keys for web registration
constexpr const char* REGISTRATION_CODE_KEY   = "registration_code";
constexpr const char* REGISTRATION_URL_KEY    = "registration_url";

// JSON keys for settings
constexpr const char* SETTINGS_ROOT_KEY  = "Settings";
constexpr const char* PRINT_FILE_SETTING = "PrintFile";
constexpr const char* JOB_ID_SETTING     = "JobID";

#endif    // SHARED_H

