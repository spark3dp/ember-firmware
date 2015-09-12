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

#define VERSION_MAJOR "2"
#define VERSION_MINOR "1"

// named pipes
#define COMMAND_PIPE            ("/tmp/CommandPipe")
#define STATUS_TO_WEB_PIPE      ("/tmp/StatusToWebPipe")

// path to file containing all current smith settings
#define ROOT_DIR                "/var/smith"
#define SETTINGS_SUB_DIR        "/config"
#define SETTINGS_FILE           "/settings"
#define SETTINGS_PATH           (ROOT_DIR SETTINGS_SUB_DIR SETTINGS_FILE)

// path to print settings file containing settings from web 
#define TEMP_SETTINGS_FILE      "/tmp/print_settings"

// path to file with registration values for display on front panel
// during primary registration
#define PRIMARY_REGISTRATION_INFO_FILE "/tmp/printer_registration"

// path to file with latest printer status
#define PRINTER_STATUS_FILE     "/run/printer_status"

// path to file written by smith-client, indicating Internet connection status
#define SMITH_STATE_FILE        "/var/local/smith_state"
// JSON key for Internet connection status
#define INTERNET_CONNECTED_KEY  "internet_connected"

// command strings
#define CMD_START_PRINT                     ("START")
#define CMD_CANCEL                          ("CANCEL")
#define CMD_PAUSE                           ("PAUSE")
#define CMD_RESUME                          ("RESUME")
#define CMD_RESET_PRINTER                   ("RESET")
#define CMD_REFRESH_SETTINGS                ("REFRESH")
#define CMD_APPLY_SETTINGS                  ("APPLYSETTINGS")
#define CMD_TEST                            ("TEST")
#define CMD_CAL_IMAGE                       ("CALIMAGE")
#define CMD_EXIT                            ("EXIT")
#define CMD_QUIT                            ("QUIT")
#define CMD_SHOW_PRINT_DATA_DOWNLOADING     ("SHOWPRINTDATADOWNLOADING")
#define CMD_SHOW_PRINT_DOWNLOAD_FAILED      ("SHOWPRINTDOWNLOADFAILED")
#define CMD_START_PRINT_DATA_LOAD           ("STARTPRINTDATALOAD")
#define CMD_SHOW_PRINT_DATA_LOADED          ("SHOWPRINTDATALOADED")
#define CMD_PROCESS_PRINT_DATA              ("PROCESSPRINTDATA")
#define CMD_REGISTRATION_CODE               ("DISPLAYPRIMARYREGISTRATIONCODE")
#define CMD_REGISTERED                      ("PRIMARYREGISTRATIONSUCCEEDED")
#define CMD_SHOW_WIRELESS_CONNECTING        ("SHOWWIRELESSCONNECTING")
#define CMD_SHOW_WIRELESS_CONNECTION_FAILED ("SHOWWIRELESSCONNECTIONFAILED")
#define CMD_SHOW_WIRELESS_CONNECTED         ("SHOWWIRELESSCONNECTED")

// JSON keys for PrinterStatus sent to web
#define STATE_PS_KEY            "state"
#define UISUBSTATE_PS_KEY       "ui_sub_state"
#define CHANGE_PS_KEY           "change"
#define IS_ERROR_PS_KEY         "is_error"
#define ERROR_CODE_PS_KEY       "error_code"
#define ERRNO_PS_KEY            "errno"
#define ERROR_MSG_PS_KEY        "error_message"
#define JOB_NAME_PS_KEY         "job_name"
#define JOB_ID_PS_KEY           "job_id"
#define LAYER_PS_KEY            "layer"
#define TOTAL_LAYERS_PS_KEY     "total_layers"
#define SECONDS_LEFT_PS_KEY     "seconds_left"
#define TEMPERATURE_PS_KEY      "temperature"
#define PRINT_RATING_PS_KEY     "print_rating"
#define SPARK_STATE_PS_KEY      "spark_state"
#define SPARK_JOB_STATE_PS_KEY  "spark_job_state"
#define LOCAL_JOB_UUID_PS_KEY   "spark_local_job_uuid"

// StaeChange enum names
#define NO_CHANGE               "none"
#define ENTERING                "entering"
#define LEAVING                 "leaving"

// Print feedback enum names
#define UNKNOWN_PRINT_FEEDBACK  "unknown"
#define PRINT_SUCCESSFUL        "successful"
#define PRINT_FAILED            "failed"

// PrinterStateMachine state names
#define PRINTER_ON_STATE               ("PrinterOn")
#define DOOR_CLOSED_STATE              ("DoorClosed")
#define INITIALIZING_STATE             ("Initializing")
#define DOOR_OPEN_STATE                ("DoorOpen")
#define HOMING_STATE                   ("Homing")
#define HOME_STATE                     ("Home")
#define ERROR_STATE                    ("Error")
#define MOVING_TO_START_POSITION_STATE ("MovingToStartPosition")
#define PRESSING_STATE                 ("Pressing")
#define PRESS_DELAY_STATE              ("PressDelay")
#define UNPRESSING_STATE               ("Unpressing")
#define PRE_EXPOSURE_DELAY_STATE       ("PreExposureDelay")
#define EXPOSING_STATE                 ("Exposing")
#define PRINTING_STATE                 ("Printing")
#define PRINTING_LAYER_STATE           ("PrintingLayer")
#define MOVING_TO_PAUSE_STATE          ("MovingToPause")
#define PAUSED_STATE                   ("Paused")
#define MOVING_TO_RESUME_STATE         ("MovingToResume")
#define SEPARATING_STATE               ("Separating")
#define APPROACHING_STATE              ("Approaching")
#define GETING_FEEDBACK_STATE          ("GettingFeedback")
#define CONFIRM_CANCEL_STATE           ("ConfirmCancel")
#define AWAITING_CANCELATION_STATE     ("AwaitingCancelation")
#define SHOWING_VERSION_STATE          ("ShowingVersion")
#define CALIBRATING_STATE              ("Calibrating")
#define REGISTERING_STATE              ("Registering")
#define UNJAMMING_STATE                ("Unjamming")
#define JAMMED_STATE                   ("Jammed")
#define DEMO_MODE_STATE                ("DemoMode")

// PrinterStateMachine UI sub-state names
#define NO_SUBSTATE                     ("NoUISubState")
#define NO_PRINT_DATA_SUBSTATE          ("NoPrintData")
#define DOWNLOADING_PRINT_DATA_SUBSTATE ("DownloadingPrintData")
#define PRINT_DOWNLOAD_FAILED_SUBSTATE  ("PrintDownloadFailed")
#define LOADING_PRINT_DATA_SUBSTATE     ("LoadingPrintData")
#define LOADED_PRINT_DATA_SUBSTATE      ("LoadedPrintData")
#define PRINT_DATA_LOAD_FAILED_SUBSTATE ("PrintDataLoadFailed")
#define HAVE_PRINT_DATA_SUBSTATE        ("HavePrintData")
#define PRINT_CANCELED_SUBSTATE         ("PrintCanceled")
#define PRINT_COMPLETED_SUBSTATE        ("PrintCompleted")
#define EXITING_DOOR_OPEN_SUBSTATE      ("ExitingDoorOpen")
#define REGISTERED_SUBSTATE             ("Registered")
#define ABOUT_TO_PAUSE_SUBSTATE         ("AboutToPause")
#define WIFI_CONNECTING_SUBSTATE        ("WiFiConnecting")
#define WIFI_CONNECTION_FAILED_SUBSTATE ("WiFiConnectionFailed")
#define WIFI_CONNECTED_SUBSTATE         ("WiFiConnected")
#define CALIBRATE_PROMPT_SUBSTATE       ("CalibratePrompt")
#define USB_FILE_FOUND_SUBSTATE         ("USBDriveFileFound")
#define USB_DRIVE_ERROR_SUBSTATE        ("USBDriveError")

// JSON keys for web registration
#define REGISTRATION_CODE_KEY   ("registration_code")
#define REGISTRATION_URL_KEY    ("registration_url")

// JSON keys for settings
#define SETTINGS_ROOT_KEY       "Settings"
#define PRINT_FILE_SETTING      "PrintFile"
#define JOB_ID_SETTING          "JobID"

#endif    // SHARED_H

