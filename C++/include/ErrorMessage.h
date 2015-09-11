//  File:   ErrorMessage.h
//  Provides a way to associate error messages with error codes
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

#ifndef ERRORMESSAGE_H
#define	ERRORMESSAGE_H

#include <iostream>
#include <stdio.h>
#include <syslog.h>
#include <sstream>
#include <cstring>

#define ERR_MSG ErrorMessage::GetMessage
#define SHORT_ERR_MSG ErrorMessage::GetShortMessage

enum ErrorCode
{
    Success = 0,
    
    SerialNumAccess = 1,
    I2cFileOpen = 2, 
    I2cSlaveAddress = 3,
    I2cWrite = 4,
    I2cLongString = 5,
    I2cReadWrite = 6,
    I2cReadRead = 7,
    GpioExport = 8,
    GpioDirection = 9,
    GpioEdge = 10,
    GpioInterrupt = 11,
    GpioInput = 12,
    FileDescriptorInUse = 13,
    NoFileDescriptor = 14,
    EpollCreate = 15,
    EpollSetup = 16,
    NegativeNumFiles = 17,
    UnexpectedEvent = 18,
    GpioUnexport = 19,
    InvalidInterrupt = 20,
    UnknownEventType = 21,
    ExposureTimerCreate = 22, // no longer used
    MotorTimerCreate = 23,    // no longer used
    MotorTimeoutTimer = 24,
    ExposureTimer = 25,
    StatusPipeCreation = 26,
    CommandPipeCreation = 27,
    WebCommandPipeCreation = 28,
    CommandResponsePipeCreation = 29,
    StatusToWebPipeOpen = 30,
    UnknownTextCommand = 31,
    PrinterStatusToString = 32,
    SendStringToPipeError = 33, 
    MotorTimeoutError = 34,
    FrontPanelError = 35,
    MotorError = 36,
    UnexpectedMotionEnd = 37,   // no longer used
    UnknownMotorEvent = 38,     // no longer used
    MotorControllerError = 39,
    UnknownFrontPanelStatus = 40,
    HardwareNeeded = 41,
    UnknownCommandInput = 42,
    RemainingExposure = 43,
    SdlInit = 44,
    SdlSetMode = 45,
    SdlHideCursor = 46,
    LoadImageError = 47,
    NoImageForLayer = 48,
    CantShowImage = 49,
    CantShowBlack = 50,
    CantGetSetting = 51,
    CantSetSetting = 52,
    CantLoadSettings = 53,
    CantRestoreSettings = 54,
    CantSaveSettings = 55,
    CantWriteSettingsString = 56,
    CantReadSettingsString = 57,
    NoDefaultSetting = 58,
    UnknownSetting = 59,
    NoValidPrintDataAvailable = 60,
    SettingOutOfRange = 61,     // no longer used
    CantStageIncomingPrintData = 62,
    InvalidPrintData = 63,
    CantLoadSettingsForPrintData = 64,
    CantMovePrintData = 65,
    CantRemovePrintData = 66, // no longer used
    IllegalStateForUISubState = 67,
    UnknownPrintEngineState = 68,
    FrontPanelNotReady = 69,
    LongFrontPanelString = 70,
    UnknownErrorCode = 71,
    UnknownPrintEngineSubState = 72,
    CantReadRegistrationInfo = 73,
    CantLoadSettingsFile = 74,
    TemperatureTimerCreate = 75, // no longer used
    TemperatureTimerError = 76,
    OverHeated = 77,
    CantOpenThermometer = 78,
    CantOpenCapeManager = 79,
    MotorSettingError = 80,     // no longer used
    WrongTypeForSetting = 81,
    SaveStatusToFileError = 82,
    IPAddressAccess = 83,
    CantOpenSocket = 84,
    CantGetWiFiMode = 85,
    UnknownUISubState = 86,
    UnknownSparkStatus = 87,
    UnknownSparkJobStatus = 88,
    CantOpenUUIDFile = 89,
    DelayTimerCreate = 90, // no longer used
    PreExposureDelayTimer = 91,
    UnknownMotorCommand = 92,
    RemainingMotorTimeout = 93,
    DuplicateLayerParams = 94,
    DuplicateLayerParamsColumn = 95,
    ZeroInMotorCommand = 96,
    NegativeInMotorCommand = 97,
    CantRestorePrintSettings = 98,
    CantDetermineConnectionStatus = 99,
    CommandPipeOpenForReading = 100,
    CommandPipeOpenForWriting = 101,
    TimerCreate = 102,
    SignalMask = 103,
    SignalfdCreate = 104,
    UdevCreate = 105,
    UdevMonitorCreate = 106,
    UdevAddFilter = 107,
    UdevMonitorEnable = 108,
    UdevGetFileDescriptor = 109,
    UsbDriveMount = 110,
    EventfdCreate = 111,
    
    // Guardrail for valid error codes
    MaxErrorCode
};

class ErrorMessage {
public:
    // Get a long error message for logging and showing in a terminal window.
    static const char* GetMessage(ErrorCode errorCode)
    {
        static bool initialized = false;
        static const char* messages[MaxErrorCode];
        if (!initialized)
        {
            // initialize the array of (long) error messages
            messages[Success] = "Success";
            
            messages[SerialNumAccess] = "Can't access board serial number";
            messages[I2cFileOpen] = "Couldn't open file in I2C_Device constructor";
            messages[I2cSlaveAddress] = "Couldn't set slave address in I2C_Device constructor";
            messages[I2cWrite] = "Error in I2C_Device::Write";
            messages[I2cLongString] = "String too long for I2C_Device::Write";
            messages[I2cReadWrite] = "Write error in I2C_Device::Read";
            messages[I2cReadRead] = "Read error in I2C_Device::Read";
            messages[GpioExport] = "Unable to export GPIO pin for %d";
            messages[GpioDirection] = "Unable to open direction handle for %d";
            messages[GpioEdge] = "Unable to open edge handle for %d";
            messages[GpioInterrupt] = "Unable to create interrupt %d";
            messages[GpioInput] = "Unable to open input for %d";
            messages[FileDescriptorInUse] = "File descriptor for %d already defined";
            messages[NoFileDescriptor] = "No file descriptor defined for subscription to event type %d";
            messages[EpollCreate] = "Couldn't create the epoll set";
            messages[EpollSetup] = "Couldn't set up epoll for %d";
            messages[NegativeNumFiles] = "Negative number of file descriptors %d";
            messages[UnexpectedEvent] = "Unexpected event from event type %d";
            messages[GpioUnexport] = "Unable to open GPIO pin for unexport";         
            messages[InvalidInterrupt] = "Invalid interrupt event type %d";
            messages[UnknownEventType] = "Unknown event type %d";
            messages[DelayTimerCreate] = "Unable to create delay timer";            
            messages[ExposureTimerCreate] = "Unable to create exposure timer";
            messages[MotorTimerCreate] = "Unable to create motor timeout timer";
            messages[MotorTimeoutTimer] = "Unable to set motor timeout timer";
            messages[PreExposureDelayTimer] = "Unable to set pre-exposure delay timer";
            messages[ExposureTimer] = "Unable to set exposure timer";
            messages[TemperatureTimerCreate] = "Unable to create thermometer timer";
            messages[TemperatureTimerError] = "Unable to set thermometer timer";
            messages[StatusPipeCreation] = "Error creating named pipe used for printer status";
            messages[CommandPipeCreation] = "Error creating named pipe used for command input";
            messages[WebCommandPipeCreation] = "Error creating named pipe used for web command input";
            messages[CommandResponsePipeCreation] = "Error creating named pipe for responding to commands";
            messages[StatusToWebPipeOpen] = "Error opening named pipe for pushing status to web";
            messages[UnknownTextCommand] = "Unknown command text: '%s'";
            messages[PrinterStatusToString] = "Can't convert printer status to JSON string";
            messages[SendStringToPipeError] = "Network interface can't send data";         
            messages[MotorTimeoutError] = "Timeout waiting for motor response, status: %d";
            messages[FrontPanelError] = "Front panel error";
            messages[MotorError] = "Motor error";
            messages[UnexpectedMotionEnd] = "End of motion when none expected";
            messages[UnknownMotorEvent] = "Unknown pending motor event: %d";
            messages[MotorControllerError] = "Motor controller error: %d";
            messages[UnknownFrontPanelStatus] = "Unknown front panel status: %d";
            messages[HardwareNeeded] = "Release build must have hardware!";
            messages[UnknownCommandInput] = "Unknown command input: %d";
            messages[RemainingExposure] = "Error reading remaining exposure time";
            messages[SdlInit] = "Could not initialize screen, SDL error: %s";
            messages[SdlSetMode] = "Could not set video mode, SDL error: %s";
            messages[SdlHideCursor] = "Could not hide cursor, SDL error: %s"; 
            messages[LoadImageError] = "Error loading image: %s";
            messages[NoImageForLayer] = "No image for layer %d";
            messages[CantShowImage] = "Can't show image for layer %d";
            messages[CantShowBlack] = "Can't clear the screen to black";
            messages[CantGetSetting] = "Can't get setting: %s";
            messages[CantSetSetting] = "Can't set setting: %s";
            messages[CantLoadSettings] = "Can't load settings file: %s";
            messages[CantRestoreSettings] = "Can't restore settings file: %s";
            messages[CantSaveSettings] = "Can't save settings file: %s";
            messages[CantWriteSettingsString] = "Can't write settings to a text string";
            messages[CantReadSettingsString] = "Can't read settings from a text string";
            messages[NoDefaultSetting] = "There is no default value for setting: %s";
            messages[UnknownSetting] = "There is no known setting named: %s";
            messages[NoValidPrintDataAvailable] = "There is no valid data available to be printed";
            messages[SettingOutOfRange] = "Setting %s out of range";
            messages[CantStageIncomingPrintData] = "Error encountered staging incoming print data";
            messages[InvalidPrintData] = "Print data invalid for file: %s";
            messages[CantLoadSettingsForPrintData] = "Error loading settings for print file: %s";
            messages[CantMovePrintData] = "Error moving print data from staging directory to print data directory for file: %s";
            messages[CantRemovePrintData] = "Error removing contents of print data directory";
            messages[IllegalStateForUISubState] = "Printer must be in Home state to change its UI sub-state, was in state %s";
            messages[UnknownPrintEngineState] = "Unknown print engine state: %d";
            messages[UnknownUISubState] = "Unknown UI sub-state: %d";
            messages[FrontPanelNotReady] = "Timeout awaiting front panel ready";
            messages[LongFrontPanelString] = "String too long (%d) to be shown on front panel";
            messages[UnknownPrintEngineSubState] = "Unknown print engine UI sub-state: %d";
            messages[CantReadRegistrationInfo] = "Can't read the registration info file from file: %s";
            messages[CantLoadSettingsFile] = "Can't load settings file: %s";
            messages[OverHeated] = "Printer temperature (%sC) is too high";
            messages[CantOpenThermometer] = "Can't find temperature sensor file";
            messages[CantOpenCapeManager] = "Can't open cape manager slots file: %s";
            messages[MotorSettingError] = "Error sending motor setting";
            messages[WrongTypeForSetting] = "Incorrect type for setting named: %s";
            messages[SaveStatusToFileError] = "Unable to save printer status to file";
            messages[IPAddressAccess] = "Can't get network interface addresses";
            messages[CantOpenSocket] = "Can't open socket to determine WiFi mode";
            messages[CantGetWiFiMode] = "Can't determine WiFi mode";
            messages[UnknownSparkStatus] = "No Spark printer status defined for key: 0x%X";
            messages[UnknownSparkJobStatus] = "No Spark job status defined for key: 0x%X";
            messages[CantOpenUUIDFile] = "Can't open UUID file: %s";
            messages[UnknownMotorCommand] = "Unknown motor command: %d";
            messages[RemainingMotorTimeout] = "Error reading remaining motor timeout time";
            messages[DuplicateLayerParams] = "Duplicate entries in CSV file for layer %d";
            messages[DuplicateLayerParamsColumn] = "Duplicate columns in CSV file for %s";
            messages[ZeroInMotorCommand] = "Zero value passed into motor command: %s";
            messages[NegativeInMotorCommand] = "Negative value passed into motor command: %s";
            messages[CantRestorePrintSettings] = "Can't restore print settings in file: %s";
            messages[CantDetermineConnectionStatus] = "Can't determine if printer is connected to Internet";
            messages[CommandPipeOpenForReading] = "Unable to open command pipe for reading";
            messages[CommandPipeOpenForWriting] = "Unable to open command pipe for writing";
            messages[TimerCreate] = "Unable to create timer";
            messages[SignalMask] = "Unable to change existing signal mask";
            messages[SignalfdCreate] = "Unable to create signalfd file descriptor";
            messages[UdevCreate] = "Unable to create udev library context";
            messages[UdevMonitorCreate] = "Unable to create udev monitor";
            messages[UdevAddFilter] = "Unable to add udev filter";
            messages[UdevMonitorEnable] = "Unable to bind udev monitor socket to the event source";
            messages[UdevGetFileDescriptor] = "Unable to retrieve the socket file descriptor associated with the udev monitor";
            messages[UsbDriveMount] = "Unable to mount usb drive (%s)";
            messages[EventfdCreate] = "Unable to create eventfd object for use with printer status queue";

            messages[UnknownErrorCode] = "Unknown error code: %d";
            initialized = true;
        }

        if (errorCode < Success ||  errorCode >= MaxErrorCode)
        {
            // don't use LOGGER here, to avoid recursion
            char buf[255];
            sprintf(buf, messages[UnknownErrorCode], errorCode);
            syslog(LOG_WARNING, buf);
            std::cerr << buf << std::endl;
            return "";                                                              
        }
        return messages[errorCode];    
    }    
    
    // Get a short error message for display where space is limited, e.g. on 
    // the front panel.
    static const char* GetShortMessage(ErrorCode errorCode)
    {
        static bool initialized = false;
        static const char* messages[MaxErrorCode];
        if (!initialized)
        {
            // initialize the array of short error messages
            for (ErrorCode ec = Success; ec < MaxErrorCode; 
                                         ec = (ErrorCode)(((int)ec) + 1))
            {
                // error codes with out short messages are initialized empty
                messages[ec] = "";
            }
                        
            messages[GpioInput] = "Door sensor";
            messages[MotorTimeoutTimer] = "Motor timer";
            messages[ExposureTimer] = "Exposure timer";
            messages[MotorTimeoutError] = "Motor timeout";
            messages[MotorError] = "Motion control";
            messages[MotorSettingError] = "Motion control";
            messages[UnexpectedMotionEnd] = "Motion control";
            messages[RemainingExposure] = "Exposure control";
            messages[NoImageForLayer] = "Missing layer image";
            messages[CantShowImage] = "Image projection";
            messages[CantShowBlack] = "Image clearing";
            messages[CantGetSetting] = "Access to setting";
            messages[CantLoadSettings] = "Loading settings";
            messages[CantRestoreSettings] = "Restoring settings";
            messages[CantRestorePrintSettings] = "Restoring settings";
            messages[CantSaveSettings] = "Saving settings";
            messages[CantReadSettingsString] = "Reading settings";
            messages[NoDefaultSetting] = "Default setting";
            messages[UnknownSetting] = "Unknown setting";
            messages[SettingOutOfRange] = "Setting range";
            messages[WrongTypeForSetting] = "Unknown setting";
            messages[OverHeated] = "Too hot, turn off!";
            messages[NoValidPrintDataAvailable] = "Invalid print data";
            initialized = true;
        }

        if (errorCode < Success || errorCode >= MaxErrorCode)
        {
            // this error will already have been logged, when attempting
            // to access the corresponding long error message
            return "";                                                              
        }
        return messages[errorCode];    
    }

    static std::string Format(ErrorCode errorCode, int value, int errnum)
    {
        char buffer[1024];
        sprintf(buffer, GetMessage(errorCode), value);
        std::ostringstream message;
        message << buffer << ": " << std::strerror(errnum);
        return message.str();
    }

    static std::string Format(ErrorCode errorCode, const char* str)
    {
        char buffer[1024];
        sprintf(buffer, GetMessage(errorCode), str);
        return std::string(buffer);
    }

    static std::string Format(ErrorCode errorCode, int errnum)
    {
        std::ostringstream message;
        message << GetMessage(errorCode) << ": " << std::strerror(errnum);
        return message.str();
    }
};

#endif    // ERRORMESSAGE_H

