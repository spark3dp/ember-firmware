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

// JSON keys for registration
#define REGISTRATION_CODE_KEY ("registration_code")
#define REGISTRATION_URL_KEY  ("registration_url")

// command strings
#define CMD_START_PRINT         ("START")
#define CMD_CANCEL              ("CANCEL")
#define CMD_PAUSE               ("PAUSE")
#define CMD_RESUME              ("RESUME")
#define CMD_RESET_PRINTER       ("RESET")
#define CMD_GET_STATUS          ("GETSTATUS")
#define CMD_REFRESH_SETTINGS    ("REFRESH")
#define CMD_TEST                ("TEST")
#define CMD_GET_FW_VERSION      ("GETFWVERSION")
#define CMD_GET_BOARD_NUM       ("GETBOARDNUM")
#define CMD_EXIT                ("EXIT")
#define CMD_QUIT                ("QUIT")
#define CMD_PRINT_DATA_LOAD     ("STARTPRINTDATALOAD")
#define CMD_PROCESS_PRINT_DATA  ("PROCESSPRINTDATA")
#define CMD_REGISTRATION_CODE   ("DISPLAYPRIMARYREGISTRATIONCODE")
#define CMD_REGISTERED          ("PRIMARYREGISTRATIONSUCCEEDED")

// JSON keys for PrinterStatus
#define PRINTER_STATUS_KEY      "PrinterStatus"
#define STATE_PS_KEY            "State"
#define CHANGE_PS_KEY           "Change"
#define IS_ERROR_PS_KEY         "IsError"
#define ERROR_CODE_PS_KEY       "ErrorCode"
#define ERRNO_PS_KEY            "Errno"
#define LAYER_PS_KEY            "Layer"
#define TOAL_LAYERS_PS_KEY      "TotalLayers"
#define SECONDS_LEFT_PS_KEY     "SecondsLeft"
#define TEMPERATURE_PS_KEY      "Temperature"
#define UISUBSTATE_PS_KEY       "UISubState"


#endif	/* SHARED_H */

