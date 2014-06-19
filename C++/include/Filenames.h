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

#define PRINTER_STATUS_PIPE ("/tmp/PrinterStatusPipe")
#define COMMAND_PIPE ("/tmp/CommandPipe")
#define STATUS_TO_WEB_PIPE ("/tmp/StatusToWebPipe")
#define LATEST_STATUS_JSON ("/tmp/StatusJSON")

#define EXECUTABLE_FOLDER "/smith"
#define IMAGE_SUB_FOLDER "/img"
#define IMAGE_FOLDER (EXECUTABLE_FOLDER IMAGE_SUB_FOLDER)
#define IMAGE_EXTENSION "png"
#define IMAGE_FILE_FILTER (EXECUTABLE_FOLDER IMAGE_SUB_FOLDER  "/*."  IMAGE_EXTENSION)

#define TEST_PATTERN_FILE "/TestPattern.png"
#define TEST_PATTERN (EXECUTABLE_FOLDER TEST_PATTERN_FILE)

#define SETTINGS_SUB_FOLDER "/config"
#define SETTINGS_FOLDER EXECUTABLE_FOLDER SETTINGS_SUB_FOLDER
#define SETTINGS_FILE "/Settings"
#define SETTINGS (SETTINGS_FOLDER SETTINGS_FILE)

#endif	/* FILENAMES_H */

