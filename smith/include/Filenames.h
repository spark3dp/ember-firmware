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

#define IMAGE_FOLDER "/smith/img"
#define IMAGE_EXTENSION "png"
#define IMAGE_FILE_FILTER (IMAGE_FOLDER  "/*."  IMAGE_EXTENSION)

#endif	/* FILENAMES_H */

