//  File:   Filenames.h
//  Names of files used by Ember
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

#ifndef FILENAMES_H
#define	FILENAMES_H

#include <Shared.h>

#define CAPE_MANAGER_SLOTS_FILE ("/sys/devices/bone_capemgr.9/slots")
#define BOARD_SERIAL_NUM_FILE ("/sys/bus/i2c/devices/0-0050/eeprom")
#define THERMOMETER_FILE ("/mnt/1wire/22.*/temperature")
#define GPIO_VALUE ("/sys/class/gpio/gpio%d/value")
#define GPIO_DIRECTION ("/sys/class/gpio/gpio%d/direction")
#define GPIO_EDGE ("/sys/class/gpio/gpio%d/edge")
#define GPIO_EXPORT ("/sys/class/gpio/export")
#define GPIO_UNEXPORT ("/sys/class/gpio/unexport")
#define UUID_FILE "/proc/sys/kernel/random/uuid"

#define SLICE_IMAGE_EXTENSION "png"
#define SLICE_IMAGE_FILE_FILTER ("/*." SLICE_IMAGE_EXTENSION)

#define PRINT_FILE_FILTER_TARGZ "/*.tar.gz"
#define PRINT_FILE_FILTER_ZIP "/*.zip"

#define SLICE_IMAGE_PREFIX "slice_"

#define TEST_PATTERN_FILE "/TestPattern.png"
#define TEST_PATTERN (ROOT_DIR TEST_PATTERN_FILE)

#define CAL_IMAGE_FILE "/Calibration.png"
#define CAL_IMAGE (ROOT_DIR CAL_IMAGE_FILE)

#define EMBEDDED_PRINT_SETTINGS_FILE "printsettings"
#define PER_LAYER_SETTINGS_FILE "layersettings.csv"

#define USB_DRIVE_MOUNT_POINT "/mnt/usb"

// name of file or directory in print data directory containing currently loaded
// print data
#define PRINT_DATA_NAME "print"

#endif    // FILENAMES_H
