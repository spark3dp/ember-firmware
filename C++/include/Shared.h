/* 
 * File:   Shared.h
 * Author: Richard Greene
 *
 * Definitions shared between different firmware & software components.
 * 
 * Created on September 25, 2014, 11:27 AM
 */

#ifndef SHARED_H
#define	SHARED_H

#define FIRMWARE_VERSION "0.2.1015.1"

// named pipes
#define COMMAND_PIPE ("/tmp/CommandPipe")
#define COMMAND_RESPONSE_PIPE ("/tmp/CommandResponsePipe")
#define STATUS_TO_WEB_PIPE ("/tmp/StatusToWebPipe")

// path to print settings file containing settings from web 
#define TEMP_PRINT_SETTINGS_FILE "/tmp/print_settings"

// path to file with registration values for display on front panel
// during primary registration
#define PRIMARY_REGISTRATION_INFO_FILE "/tmp/printer_registration"

// command strings
#define CMD_START_PRINT         ("START")
#define CMD_CANCEL              ("CANCEL")
#define CMD_PAUSE               ("PAUSE")
#define CMD_RESUME              ("RESUME")
#define CMD_RESET_PRINTER       ("RESET")
#define CMD_GET_STATUS          ("GETSTATUS")
#define CMD_REFRESH_SETTINGS    ("REFRESH")
#define CMD_APPLY_PRINT_SETTINGS ("APPLYPRINTSETTINGS")
#define CMD_TEST                ("TEST")
#define CMD_GET_FW_VERSION      ("GETFWVERSION")
#define CMD_GET_BOARD_NUM       ("GETBOARDNUM")
#define CMD_EXIT                ("EXIT")
#define CMD_QUIT                ("QUIT")
#define CMD_PRINT_DATA_LOAD     ("STARTPRINTDATALOAD")
#define CMD_PROCESS_PRINT_DATA  ("PROCESSPRINTDATA")
#define CMD_REGISTRATION_CODE   ("DISPLAYPRIMARYREGISTRATIONCODE")
#define CMD_REGISTERED          ("PRIMARYREGISTRATIONSUCCEEDED")
#define CMD_CALIBRATE           ("CALIBRATE")

// JSON keys for PrinterStatus
#define STATE_PS_KEY            "state"
#define CHANGE_PS_KEY           "change"
#define IS_ERROR_PS_KEY         "is_error"
#define ERROR_CODE_PS_KEY       "error_code"
#define ERRNO_PS_KEY            "errno"
#define LAYER_PS_KEY            "layer"
#define TOAL_LAYERS_PS_KEY      "total_layers"
#define SECONDS_LEFT_PS_KEY     "seconds_left"
#define TEMPERATURE_PS_KEY      "temperature"
#define UISUBSTATE_PS_KEY       "ui_sub_state"

// PrinterStateMachine state names
#define PRINTER_ON_STATE        ("PrinterOn")
#define DOOR_CLOSED_STATE       ("DoorClosed")
#define INITIALIZING_STATE      ("Initializing")
#define DOOR_OPEN_STATE         ("DoorOpen")
#define HOMING_STATE            ("Homing")
#define HOME_STATE              ("Home")
#define IDLE_STATE              ("Idle")
#define PRINT_SETUP_STATE       ("PrintSetup")
#define MOVING_TO_START_POSITION_STATE ("MovingToStartPosition")
#define EXPOSING_STATE          ("Exposing")
#define PRINTING_STATE          ("Printing")
#define PRINTING_LAYER_STATE    ("PrintingLayer")
#define PAUSED_STATE            ("Paused")
#define SEPARATING_STATE        ("Separating")
#define ENDING_PRINT_STATE      ("EndingPrint")
#define CONFIRM_CANCEL_STATE    ("ConfirmCancel")
#define SHOWING_VERSION_STATE   ("ShowingVersion")
#define CALIBRATE_STATE         ("Calibrate")
#define MOVING_TO_CALIBRATION_STATE ("MovingToCalibration")
#define CALIBRATING_STATE       ("Calibrating")
#define REGISTERING_STATE       ("Registering")
#define REGISTERED_STATE        ("Registered")

// PrinterStateMachine UI substate names
#define NO_SUBSTATE                 ("NoUISubState")
#define NO_PRINT_DATA_SUBSTATE      ("NoPrintData")
#define DOWNLOADING_SUBSTATE        ("Downloading")
#define DOWNLOADED_SUBSTATE         ("Downloaded")
#define DOWNLOAD_FAILED_SUBSTATE    ("DownloadFailed")
#define HAVE_PRINT_DATA_SUBSTATE    ("HavePrintData")
#define PRINT_CANCELED_SUBSTATE     ("PrintCanceled")
#define PRINT_COMPLETED_SUBSTATE    ("PrintCompleted")
#define EXITING_DOOR_OPEN_SUBSTATE  ("ExitingDoorOpen")

// JSON keys for web registration
#define REGISTRATION_CODE_KEY ("registration_code")
#define REGISTRATION_URL_KEY  ("registration_url")

// JSON keys for settings
#define SETTINGS_ROOT_KEY "Settings"

#endif	/* SHARED_H */

