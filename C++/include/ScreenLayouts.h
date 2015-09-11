//  File:   ScreenLayouts.h
//  Defines the layout of the front panel screens
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Shalom Ormsby
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

#include <Version.h>

//      ID                        Alignment     X       Y    Size    Color      Text
#define READY_LOADED_LINE1          Center,     64,     16,     1,  0xFFFF,     "Ready."
#define READY_LOADED_LINE2          Center,     64,     32,     1,  0xFFFF,     "Load your prepped"
#define READY_LOADED_LINE3          Center,     64,     48,     1,  0xFFFF,     "print file via"
#define READY_LOADED_LINE4          Center,     64,     64,     1,  0xFFFF,     "network or USB."
#define READY_LOADED_BTN1_LINE1     Left,       0,      96,     1,  0xFFFF,     "Clear"
#define READY_LOADED_BTN1_LINE2     Left,       0,      112,    1,  0xFFFF,     "file"
#define READY_LOADED_BTN2_LINE1     Right,      127,    96,     1,  0xFFFF,     "Reprint"
#define READY_LOADED_BTN2_LINE2     Right,      127,    112,    1,  0xFFFF,     "file"
#define READY_LOADED_LED_SEQ    0

#define START_LOADED_LINE1          Center,     64,     8,      1,  0xFFFF,     "%s"
#define START_LOADED_LINE2          Center,     64,     24,     1,  0xFFFF,     "loaded. Is the build"
#define START_LOADED_LINE3          Center,     64,     40,     1,  0xFFFF,     "head ready? Is"
#define START_LOADED_LINE4          Center,     64,     56,     1,  0xFFFF,     "there enough resin?"
#define START_LOADED_LINE5          Center,     64,     72,     1,  0xFFFF,     "If yes, press Start." 
#define START_LOADED_BTN1_LINE2     Left,       0,      112,    1,  0xFFFF,     "Clear"
#define START_LOADED_BTN2_LINE2     Right,      127,    112,    1,  0xFFFF,     "Start"
#define START_LOADED_LED_SEQ    8

#define LOAD_FAIL_LINE1             Center,     64,     32,     1,  0xFFFF,     "Error"
#define LOAD_FAIL_LINE2             Center,     64,     48,     1,  0xFFFF,     "loading file."
#define LOAD_FAIL_BTN2_LINE2        Right,      127,    112,    1,  0xFFFF,     "Okay"
#define LOAD_FAIL_LED_SEQ   4

#define DOWNLOAD_FAIL_LINE1         Center,     64,     32,     1,  0xFFFF,     "Error"
#define DOWNLOAD_FAIL_LINE2         Center,     64,     48,     1,  0xFFFF,     "downloading file."
#define DOWNLOAD_FAIL_BTN2_LINE2    Right,      127,    112,    1,  0xFFFF,     "Okay"
#define DOWNLOAD_FAIL_LED_SEQ   4

#define PRINTING_LINE1              Center,     64,     8,      1,  0xFFFF,     "Printing"
#define PRINTING_LINE2              Center,     64,     24,     1,  0xFFFF,     "%s" // job name
#define PRINTING_CLEAR_LINE3        Center,     64,     46,     2,  0x0000,     "%s" // clears previous time
#define PRINTING_LINE3              Center,     64,     46,     2,  0xFFFF,     "%s" // shows new time
#define PRINTING_LINE4              Center,     64,     88,     1,  0xFFFF,     "remaining."
#define PRINTING_BTN1_LINE2         Left,       0,      112,    1,  0xFFFF,     "Cancel"
#define PRINTING_BTN2_LINE2         Right,      127,    112,    1,  0xFFFF,     "Pause"
#define PRINTING_LED_SEQ    0

#define ABOUT_TO_PAUSE_LINE1        Center,     64,     32,     1,  0xFFFF,     "Pausing..."
#define ABOUT_TO_PAUSE_LED_SEQ 9

#define ABOUT_TO_RESUME_LINE1       Center,     64,     32,     1,  0xFFFF,     "Resuming..."
#define ABOUT_TO_RESUME_LED_SEQ 9

#define PAUSED_LINE1                Center,     64,     32,     1,  0xFFFF,     "Print"
#define PAUSED_LINE2                Center,     64,     48,     1,  0xFFFF,     "paused."
#define PAUSED_BTN1_LINE1           Left,       0,      96,     1,  0xFFFF,     "Cancel"
#define PAUSED_BTN1_LINE2           Left,       0,      112,    1,  0xFFFF,     "print"
#define PAUSED_BTN2_LINE2           Right,      127,    112,    1,  0xFFFF,     "Resume"
#define PAUSED_LED_SEQ  9

#define UNJAMMING_LINE1             Center,     64,     32,     1,  0xFFFF,     "Resin tray jammed"
#define UNJAMMING_LINE2             Center,     64,     48,     1,  0xFFFF,     "Attempting to"
#define UNJAMMING_LINE3             Center,     64,     64,     1,  0xFFFF,     "autocorrect..."
// reuses PAUSED_... left button labels
#define UNJAMMING_LED_SEQ  4

#define JAMMED_LINE1                Center,     64,     32,     1,  0xFFFF,     "Resin tray jammed"
#define JAMMED_LINE2                Center,     64,     48,     1,  0xFFFF,     "Manually rotate the"
#define JAMMED_LINE3                Center,     64,     64,     1,  0xFFFF,     "resin tray plate."
// reuses both PAUSED_... button labels
#define JAMMED_LED_SEQ  4

#define CONFIRM_CANCEL_LINE1        Center,     64,     32,     1,  0xFFFF,     "Cancel"
#define CONFIRM_CANCEL_LINE2        Center,     64,     48,     1,  0xFFFF,     "print?"
#define CONFIRM_CANCEL_BTN1_LINE1   Left,       0,      96,     1,  0xFFFF,     "Yes,"
#define CONFIRM_CANCEL_BTN1_LINE2   Left,       0,      112,    1,  0xFFFF,     "cancel"
#define CONFIRM_CANCEL_BTN2_LINE1   Right,      127,    96,     1,  0xFFFF,     "No,"
#define CONFIRM_CANCEL_BTN2_LINE2   Right,      127,    112,    1,  0xFFFF,     "resume"
#define CONFIRM_CANCEL_LED_SEQ    4

#define PRINT_COMPLETE_LINE1        Center,     64,     32,     1,  0xFFFF,     "Print complete."
#define PRINT_COMPLETE_LINE2        Center,     64,     48,     1,  0xFFFF,     "Clear the build"
#define PRINT_COMPLETE_LINE3        Center,     64,     64,     1,  0xFFFF,     "head."
#define PRINT_COMPLETE_LED_SEQ  6


#define GET_FEEDBACK_LINE1          Center,     64,     32,     1,  0xFFFF,     "Was the print" 
#define GET_FEEDBACK_LINE2          Center,     64,     48,     1,  0xFFFF,     "successful?"
#define GET_FEEDBACK_BTN1_LINE2     Left,       0,      112,    1,  0xFFFF,     "No"
#define GET_FEEDBACK_BTN2_LINE2     Right,      127,    112,    1,  0xFFFF,     "Yes"
#define GET_FEEDBACK_LED_SEQ    4

#define STARTING_PRINT_LINE1        Center,     64,     32,     1,  0xFFFF,     "Starting to print"
#define STARTING_PRINT_LINE2        Center,     64,     48,     1,  0xFFFF,     "%s"
#define STARTING_PRINT_LED_SEQ   0

#define CALIBRATE_PROMPT_LINE1      Center,     64,     80,     1,  0xFFFF,     "Skip calibration?"
#define CALIBRATE_PROMPT_BTN2_LINE2 Right,      127,    112,    1,  0xFFFF,     "Yes"
#define CALIBRATE_PROMPT_LED_SEQ   0

#define LOAD_FIRST_LINE1            Center,     64,     16,     1,  0xFFFF,     "Ready."
#define LOAD_FIRST_LINE2            Center,     64,     32,     1,  0xFFFF,     "Load your prepped"
#define LOAD_FIRST_LINE3            Center,     64,     48,     1,  0xFFFF,     "print file via"
#define LOAD_FIRST_LINE4            Center,     64,     64,     1,  0xFFFF,     "network or USB."
#define LOAD_FIRST_LED_SEQ    0

#define DOWNLOADING_FILE_LINE1      Center,     64,     32,     1,  0xFFFF,     "Downloading file..."
#define DOWNLOADING_FILE_LED_SEQ    1

#define LOADING_FILE_LINE1          Center,     64,     32,     1,  0xFFFF,     "Loading file..."
#define LOADING_FILE_LED_SEQ    1

#define CANCELED_LINE1              Center,     64,     32,     1,  0xFFFF,     "Print canceled."
#define CANCELED_LINE2              Center,     64,     48,     1,  0xFFFF,     "Clear the build"
#define CANCELED_LINE3              Center,     64,     64,     1,  0xFFFF,     "head."
#define CANCELED_LED_SEQ      7
 
#define DOOR_OPEN_LINE1             Center,     64,     8,      1,  0xFFFF,     "Door open."
#define DOOR_OPEN_LINE2             Center,     64,     24,     1,  0xFFFF,     "Please close the"
#define DOOR_OPEN_LINE3             Center,     64,     40,     1,  0xFFFF,     "door ASAP to avoid"
#define DOOR_OPEN_LINE4             Center,     64,     56,     1,  0xFFFF,     "exposing the photo-"
#define DOOR_OPEN_LINE5             Center,     64,     72,     1,  0xFFFF,     "sensitive resin."
#define DOOR_OPEN_LED_SEQ    4

#define ERROR_CODE_LINE1            Center,     64,     16,     1,  0xFFFF,     "Error."
#define ERROR_CODE_LINE2            Center,     64,     32,     1,  0xFFFF,     "%s"
#define ERROR_CODE_LINE3            Center,     64,     48,     1,  0xFFFF,     "%s"
#define ERROR_CODE_BTN1_LINE2       Left,       0,      112,    1,  0xFFFF,     "Reset"
#define ERROR_CODE_LED_SEQ  4

#define HOMING_LINE1                Center,     64,     32,     1,  0xFFFF,     "Homing the build"
#define HOMING_LINE2                Center,     64,     48,     1,  0xFFFF,     "head. Please wait."
#define HOMING_LED_SEQ  0

#define SYSINFO_LINE1               Center,     64,     8,      1,  0xFFFF,     "Firmware version"
#define SYSINFO_LINE2               Center,     64,     24,     1,  0xFFFF,     FIRMWARE_VERSION
#define SYSINFO_LINE3               Center,     64,     48,     1,  0xFFFF,     "IP address"
#define SYSINFO_LINE4               Center,     64,     64,     1,  0xFFFF,     "%s"
#define SYSINFO_BTN2_LINE2          Right,      127,    112,    1,  0xFFFF,     "Okay"
#define SYSINFO_LED_SEQ  0

#define CALIBRATING_LINE1           Center,     64,     16,     1,  0xFFFF,     "Complete calibration"
#define CALIBRATING_LINE2           Center,     64,     32,     1,  0xFFFF,     "procedure."
#define CALIBRATING_BTN2_LINE2      Right,      127,    112,    1,  0xFFFF,     "Done"
#define CALIBRATING_LED_SEQ    0

#define PAIR_ACCOUNT_LINE1          Center,     64,     16,     1,  0xFFFF,     "Internet connected."
#define PAIR_ACCOUNT_LINE2          Center,     64,     32,     1,  0xFFFF,     "Pair with Spark at"
#define PAIR_ACCOUNT_LINE3          Center,     64,     48,     1,  0xFFFF,     "%s"
#define PAIR_ACCOUNT_LINE4          Center,     64,     64,     1,  0xFFFF,     "and enter this code"
#define PAIR_ACCOUNT_LINE5          Center,     64,     80,     1,  0xFFFF,     "%s"
#define PAIR_ACCOUNT_BTN1_LINE2     Left,       0,      112,    1,  0xFFFF,     "Cancel"
#define PAIR_ACCOUNT_LED_SEQ 8

#define PAIR_SUCCESS_LINE1          Center,     64,     16,     1,  0xFFFF,     "Success!"
#define PAIR_SUCCESS_LINE2          Center,     64,     32,     1,  0xFFFF,     "Your printer is now"
#define PAIR_SUCCESS_LINE3          Center,     64,     48,     1,  0xFFFF,     "paired with Spark"
#define PAIR_SUCCESS_LINE4          Center,     64,     64,     1,  0xFFFF,     "and is ready to"
#define PAIR_SUCCESS_LINE5          Center,     64,     80,     1,  0xFFFF,     "start printing."
#define PAIR_SUCCESS_BTN2_LINE2     Right,      127,    112,    1,  0xFFFF,     "Okay"
#define PAIR_SUCCESS_LED_SEQ 5

#define WIFI_CONNECTING_LINE1       Center,     64,     32,     1,  0xFFFF,     "Attempting to"
#define WIFI_CONNECTING_LINE2       Center,     64,     48,     1,  0xFFFF,     "connect to WiFi..."
#define WIFI_CONNECTING_BTN2_LINE2  Right,      127,    112,    1,  0xFFFF,     "Okay"
#define WIFI_CONNECTING_LED_SEQ   1

#define WIFI_NOT_CONNECTED_LINE1    Center,     64,     32,     1,  0xFFFF,     "Unable to"
#define WIFI_NOT_CONNECTED_LINE2    Center,     64,     48,     1,  0xFFFF,     "connect to WiFi."
#define WIFI_NOT_CONNECTED_BTN2_LINE2 Right,    127,    112,    1,  0xFFFF,     "Okay"
#define WIFI_NOT_CONNECTED_LED_SEQ   4

#define WIFI_CONNECTED_LINE1        Center,     64,     32,     1,  0xFFFF,     "Connected to WiFi."
#define WIFI_CONNECTED_BTN2_LINE2   Right,      127,    112,    1,  0xFFFF,     "Okay"
#define WIFI_CONNECTED_LED_SEQ   5

#define UNKNOWN_SCREEN_LINE1        Center,     64,     16,     1,  0xFFFF,     "No screen for"
#define UNKNOWN_SCREEN_LINE2        Center,     64,     32,     1,  0xFFFF,     "%s"
#define UNKNOWN_SCREEN_LINE3        Center,     64,     48,     1,  0xFFFF,     "%s"
#define UNKNOWN_SCREEN_LED_SEQ  0

#define DEMO_SCREEN_LINE1           Center,     64,     16,     1,  0xFFFF,     "Autodesk Ember"
#define DEMO_SCREEN_LINE2           Center,     64,     96,     1,  0xFBA0,     "#SparkPowered"
#define DEMO_SCREEN_LED_SEQ  8
                                                    
#define USB_FILE_FOUND_LINE1        Center,     64,     16,     1,  0xFFFF,     "Do you want to load"
#define USB_FILE_FOUND_LINE2        Center,     64,     48,     1,  0xFFFF,     "%s" 
#define USB_FILE_FOUND_LINE3        Center,     64,     64,     1,  0xFFFF,     "%s"
#define USB_FILE_FOUND_LINE4        Center,     64,     80,     1,  0xFFFF,     "%s"
#define USB_FILE_FOUND_BTN1_LINE2   Left,       0,      112,    1,  0xFFFF,     "No" 
#define USB_FILE_FOUND_BTN2_LINE2   Right,      127,    112,    1,  0xFFFF,     "Yes" 
#define USB_FILE_FOUND_LED_SEQ  1    

#define USB_DRIVE_ERROR_LINE1       Center,     64,     16,     1,  0xFFFF,     "Print data not found"
#define USB_DRIVE_ERROR_LINE2       Center,     64,     32,     1,  0xFFFF,     "Place a single print" 
#define USB_DRIVE_ERROR_LINE3       Center,     64,     48,     1,  0xFFFF,     "data file in the"
#define USB_DRIVE_ERROR_LINE4       Center,     64,     64,     1,  0xFFFF,     "%s"
#define USB_DRIVE_ERROR_LINE5       Center,     64,     80,     1,  0xFFFF,     "folder of your drive"
#define USB_DRIVE_ERROR_BTN2_LINE2  Right,      127,    112,    1,  0xFFFF,     "Okay" 
#define USB_DRIVE_ERROR_LED_SEQ  4                                               
