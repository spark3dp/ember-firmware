/* 
 * File:   Filenames.h
 * Author: Richard Greene
 * 
 * Names of named pipes and other shared files.
 *
 * Created on May 19, 2014, 11:17 AM
 */

#ifndef FILENAMES_H
#define	FILENAMES_H

#define FIRMWARE_VERSION ("0.1.0.0")
#define BOARD_SERIAL_NUM_FILE ("/sys/bus/i2c/devices/0-0050/eeprom")

#define PRINTER_STATUS_PIPE ("/tmp/PrinterStatusPipe")
#define COMMAND_PIPE ("/tmp/CommandPipe")
#define COMMAND_RESPONSE_PIPE ("/tmp/CommandResponsePipe")
#define STATUS_TO_WEB_PIPE ("/tmp/StatusToWebPipe")

#define ROOT_DIR "/smith"

#define IMAGE_EXTENSION "png"
#define IMAGE_FILE_FILTER ("/*." IMAGE_EXTENSION)

#define PRINT_FILE_EXTENSION "tar.gz"
#define PRINT_FILE_FILTER ("/*." PRINT_FILE_EXTENSION)

#define TEST_PATTERN_FILE "/TestPattern.png"
#define TEST_PATTERN (ROOT_DIR TEST_PATTERN_FILE)

#define SETTINGS_FILE "/settings"
#define SETTINGS_SUB_DIR "/config"
#define SETTINGS_PATH (ROOT_DIR SETTINGS_SUB_DIR SETTINGS_FILE)

//#define IMAGE_SUB_FOLDER "/img"
//#define IMAGE_FOLDER (EXECUTABLE_FOLDER IMAGE_SUB_FOLDER)
//#define STAGING_SUB_FOLDER "/staging"
//#define STAGING_FOLDER (EXECUTABLE_FOLDER STAGING_SUB_FOLDER)
//#define DOWNLOAD_SUB_FOLDER "/download"
//#define DOWNLOAD_FOLDER (EXECUTABLE_FOLDER DOWNLOAD_SUB_FOLDER)
//#define IMAGE_FILE_FILTER (EXECUTABLE_FOLDER IMAGE_SUB_FOLDER  "/*."  IMAGE_EXTENSION)
//#define PRINT_FILE_FILTER (EXECUTABLE_FOLDER DOWNLOAD_SUB_FOLDER "/*.tar.gz")
//#define SETTINGS_SUB_FOLDER "/config"
//#define SETTINGS_FOLDER EXECUTABLE_FOLDER SETTINGS_SUB_FOLDER
//#define SETTINGS_FILE "/settings"
//#define SETTINGS_PATH (SETTINGS_FOLDER SETTINGS_FILE)

#endif	/* FILENAMES_H */

