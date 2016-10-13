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

constexpr const char* BOARD_SERIAL_NUM_FILE   = "/sys/bus/nvmem/devices/at24-0/nvmem";
constexpr const char* THERMOMETER_FILE        = "/mnt/1wire/22.*/temperature";
constexpr const char* GPIO_VALUE              = "/sys/class/gpio/gpio%d/value";
constexpr const char* GPIO_DIRECTION          = "/sys/class/gpio/gpio%d/direction";
constexpr const char* GPIO_EDGE               = "/sys/class/gpio/gpio%d/edge";
constexpr const char* GPIO_EXPORT             = "/sys/class/gpio/export";
constexpr const char* GPIO_UNEXPORT           = "/sys/class/gpio/unexport";
constexpr const char* UUID_FILE               = "/proc/sys/kernel/random/uuid";

constexpr const char* SLICE_IMAGE_PREFIX       = "slice_";
constexpr const char* SLICE_IMAGE_EXTENSION    = "png";
constexpr const char* FILE_FILTER_PREFIX       = "/*.";

constexpr const char* PRINT_FILE_FILTER_TARGZ = "/*.tar.gz";
constexpr const char* PRINT_FILE_FILTER_ZIP   = "/*.zip";
constexpr const char* PRINT_FILE_FILTER_ANY   = "/*";

constexpr const char* TEST_PATTERN_FILE = "/TestPattern.png";
constexpr const char* CAL_IMAGE_FILE    = "/Calibration.png";

constexpr const char* EMBEDDED_PRINT_SETTINGS_FILE = "printsettings";
constexpr const char* PER_LAYER_SETTINGS_FILE      = "layersettings.csv";

constexpr const char* USB_DRIVE_MOUNT_POINT = "/mnt/usb";

// name of file or directory in print data directory containing currently loaded
// print data
constexpr const char* PRINT_DATA_NAME = "print";

constexpr const char* PROJECTOR_FW_FILE = "/lib/projector/Autodesk_3_0_no_images.bin";

constexpr const char* DRM_DEVICE_NODE = "/dev/dri/card0";
constexpr const char* MEMORY_DEVICE = "/dev/mem";

#endif    // FILENAMES_H
