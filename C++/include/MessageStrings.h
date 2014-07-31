/* 
 * File:   MessageStrings.h
 * Author: greener
 *
 * Created on March 31, 2014, 4:57 PM
 */

#ifndef MESSAGESTRINGS_H
#define	MESSAGESTRINGS_H

#define PRINTER_STARTUP_MSG ("Spark Printer (pre-release)")
#define FW_VERSION_MSG ("Firmware version: ")
#define BOARD_SER_NUM_MSG ("Serial number: ")

#define PRINTER_STATUS_FORMAT (", layer %d of %d, seconds left: %d")
#define LOG_STATUS_FORMAT ("entering %s")
#define ERROR_FORMAT "%s: %s"
#define LOG_ERROR_FORMAT (ERROR_FORMAT "\n")
#define LOG_MOTOR_EVENT ("motor interrupt: %d")
#define LOG_BUTTON_EVENT ("button interrupt: %d")
#define LOG_DOOR_EVENT ("door interrupt: %c")
#define LOG_KEYBOARD_INPUT ("keyboard input: %s")
#define LOG_UI_COMMAND ("UI command: %s")
#define LOG_WEB_COMMAND ("web command: %s")


#endif	/* MESSAGESTRINGS_H */

