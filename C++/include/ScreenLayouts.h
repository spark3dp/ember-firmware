/* 
 * File:   ScreenLayouts.h
 * 
 * Author: Shalom Ormsby
 *
 * Defines the layout of the front panel displays.
 * 
 * Created on July 22, 2014
 */

#include <Version.h>

//      ID                        Alignment     X       Y    Size    Color      Text
#define	READY_LOADED_LINE1          Center,     64,     16,     1, 	0xFFFF,     "Ready."
#define	READY_LOADED_LINE2          Center,     64,     32,     1, 	0xFFFF,     "Load your prepped"
#define	READY_LOADED_LINE3          Center,     64,     48,     1, 	0xFFFF,     "print file via WiFi"
#define	READY_LOADED_LINE4          Center,     64,     64,     1, 	0xFFFF,     "or USB."
#define	READY_LOADED_BTN1_LINE1     Left,       0,      96,     1, 	0xFFFF,     "Reprint"
#define	READY_LOADED_BTN1_LINE2     Left,       0,      112,    1, 	0xFFFF,     "file"
#define	READY_LOADED_BTN2_LINE1     Right,      127,    96,     1, 	0xFFFF,     "Clear"
#define	READY_LOADED_BTN2_LINE2     Right,      127,    112,    1, 	0xFFFF,     "file"
#define	READY_LOADED_LED_SEQ    0

#define	START_LOADED_LINE1          Center,     64,     8,      1, 	0xFFFF,     "\"%s\""
#define	START_LOADED_LINE2          Center,     64,     24,     1, 	0xFFFF,     "loaded. Is the"
#define	START_LOADED_LINE3          Center,     64,     40,     1, 	0xFFFF,     "build area ready? Is"
#define	START_LOADED_LINE4          Center,     64,     56,     1, 	0xFFFF,     "there enough resin?"
#define	START_LOADED_LINE5          Center,     64,     72,     1, 	0xFFFF,     "If yes, press Start." 
#define	START_LOADED_BTN1_LINE2     Left,       0,      112,    1, 	0xFFFF,     "Start"
#define	START_LOADED_BTN2_LINE2     Right,      127,    112,    1, 	0xFFFF,     "Clear"
#define	START_LOADED_LED_SEQ    8

#define	LOAD_FAIL_LINE1             Center,     64,     32,     1, 	0xFFFF,     "Error"
#define	LOAD_FAIL_LINE2             Center,     64,     48,     1, 	0xFFFF,     "loading file."
#define	LOAD_FAIL_BTN1_LINE2        Left,       0,      112,    1, 	0xFFFF,     "Okay"
#define	LOAD_FAIL_LED_SEQ   4

#define	PRINTING_LINE1              Center,     64,     8,      1, 	0xFFFF,     "Printing..."
#define PRINTING_CLEAR_LINE2        Center,     64,     32,     2, 	0x0000,     "%s"
#define	PRINTING_LINE2              Center,     64,     32,     2, 	0xFFFF,     "%s"
#define	PRINTING_LINE3              Center,     64,     80,     1, 	0xFFFF,     "remaining."
#define	PRINTING_BTN1_LINE2         Left,       0,      112,    1, 	0xFFFF,     "Pause"
#define	PRINTING_BTN2_LINE2         Right,      127,    112,    1, 	0xFFFF,     "Cancel"
#define	PRINTING_LED_SEQ    0

#define	PAUSED_LINE1                Center,     64,     32,     1, 	0xFFFF,     "Print"
#define	PAUSED_LINE2                Center,     64,     48,     1, 	0xFFFF,     "paused."
#define	PAUSED_BTN1_LINE2           Left,       0,      112,    1, 	0xFFFF,     "Resume"
#define	PAUSED_BTN2_LINE1           Right,      127,    96,     1, 	0xFFFF,     "Cancel"
#define	PAUSED_BTN2_LINE2           Right,      127,    112,    1, 	0xFFFF,     "print"
#define	PAUSED_LED_SEQ  8

#define	CONFIRM_CANCEL_LINE1        Center,     64,     32,     1, 	0xFFFF,     "Cancel"
#define	CONFIRM_CANCEL_LINE2        Center,     64,     48,     1, 	0xFFFF,     "print?"
#define	CONFIRM_CANCEL_BTN1_LINE1   Left,       0,      96,    1, 	0xFFFF,     "No,"
#define	CONFIRM_CANCEL_BTN1_LINE2   Left,       0,      112,    1, 	0xFFFF,     "resume"
#define	CONFIRM_CANCEL_BTN2_LINE1   Right,      127,    96,    1, 	0xFFFF,     "Yes,"
#define	CONFIRM_CANCEL_BTN2_LINE2   Right,      127,    112,    1, 	0xFFFF,     "cancel"
#define	CONFIRM_CANCEL_LED_SEQ    4

#define	PRINT_COMPLETE_LINE1        Center,     64,     32,     1, 	0xFFFF,     "Print complete."
#define	PRINT_COMPLETE_LINE2        Center,     64,     48,     1, 	0xFFFF,     "Clear the build"
#define	PRINT_COMPLETE_LINE3        Center,     64,     64,     1, 	0xFFFF,     "area."
#define	PRINT_COMPLETE_LED_SEQ  6

#define STARTING_PRINT_LINE1        Center,     64,     32,     1,  0xFFFF,     "Starting print..."
#define STARTING_PRINT_BTN2_LINE2   Right,      127,    112,    1,  0xFFFF,     "Cancel"
#define STARTING_PRINT_LED_SEQ   0

#define LOAD_FIRST_LINE1            Center,     64,     16,     1,  0xFFFF,     "Ready."
#define LOAD_FIRST_LINE2            Center,     64,     32,     1,  0xFFFF,     "Load your prepped"
#define LOAD_FIRST_LINE3            Center,     64,     48,     1,  0xFFFF,     "print file via WiFi"
#define LOAD_FIRST_LINE4            Center,     64,     64,     1,  0xFFFF,     "or USB."
#define LOAD_FIRST_LED_SEQ    0

#define LOADING_FILE_LINE1          Center,     64,     32,     1,  0xFFFF,     "Loading file..."
#define LOADING_FILE_LED_SEQ    0

#define	CANCELED_LINE1              Center,     64,     32,     1, 	0xFFFF,     "Print canceled."
#define	CANCELED_LINE2              Center,     64,     48,     1, 	0xFFFF,     "Clear the build"
#define	CANCELED_LINE3              Center,     64,     64,     1, 	0xFFFF,     "area."
#define	CANCELED_LED_SEQ      7
 
#define	DOOR_OPEN_LINE1             Center,     64,     8,     1, 	0xFFFF,     "Door open."
#define	DOOR_OPEN_LINE2             Center,     64,     24,     1, 	0xFFFF,     "Please close the"
#define	DOOR_OPEN_LINE3             Center,     64,     40,     1, 	0xFFFF,     "door ASAP to avoid"
#define	DOOR_OPEN_LINE4             Center,     64,     56,     1, 	0xFFFF,     "exposing the photo-"
#define	DOOR_OPEN_LINE5             Center,     64,     72,     1, 	0xFFFF,     "sensitive resin."
#define	DOOR_OPEN_LED_SEQ    4

#define	ERROR_CODE_LINE1            Center,     64,     16,     1, 	0xFFFF,     "Error."
#define	ERROR_CODE_LINE2            Center,     64,     32,     1, 	0xFFFF,     "%s"
#define	ERROR_CODE_LINE3            Center,     64,     48,     1, 	0xFFFF,     "%s"
#define	ERROR_CODE_LINE4            Center,     64,     64,     1, 	0xFFFF,     "Return home or"
#define	ERROR_CODE_LINE5            Center,     64,     80,     1, 	0xFFFF,     "reset?"
#define	ERROR_CODE_BTN1_LINE2       Left,        0,     112,    1, 	0xFFFF,     "Home"
#define ERROR_CODE_BTN2_LINE2       Right,     127,     112,    1,  0xFFFF,     "Reset"
#define	ERROR_CODE_LED_SEQ  4

#define	HOMING_LINE1                Center,     64,     32,     1, 	0xFFFF,     "Homing the build"
#define	HOMING_LINE2                Center,     64,     48,     1, 	0xFFFF,     "area. Please wait..."
#define	HOMING_LED_SEQ  0

#define	VERSION_LINE1              Center,     64,     32,     1, 	0xFFFF,     "Firmware version"
#define	VERSION_LINE2              Center,     64,     48,     1, 	0xFFFF,     FIRMWARE_VERSION
#define	VERSION_BTN1_LINE2         Left,        0,     112,    1, 	0xFFFF,     "Okay"
#define	VERSION_LED_SEQ  0




