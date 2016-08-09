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
#include <vector>

enum ErrorCode
{
    Success = 0,
    
    SerialNumAccess = 1,
    I2cFileOpen = 2, 
    I2cSlaveAddress = 3,
    I2cWrite = 4,
    I2cLongString = 5, // no longer used
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
    SdlInit = 44,       // no longer used
    SdlSetMode = 45,    // no longer used
    SdlHideCursor = 46, // no longer used
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
    CantOpenCapeManager = 79,   // no longer used
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
    SdlCreateSurface = 112, // no longer used
    IPThreadAlreadyRunning = 113,
    CantStartIPThread = 114,
    CantJoinIPThread = 115,
    ImageProcessing = 116,
    CantShowWhite = 117,
    SdlLockSurface = 118, // no longer used
    SdlFillRect = 119,    // no longer used
    SdlFlip = 120,        // no longer used
    SdlBlitSurface = 121, // no longer used
    I2cReadReadWhenReady = 122,
    I2cDeviceNotReady = 123,
    ProjectorGammaError = 124,
    PatternModeError = 125,
    ProjectorUpgradeError = 126,
    CantReadProjectorFwVersion = 127, 
    CantReadProjectorMfrID = 128,  
    UnknownProjectorMfrID = 129,   
    CantReadProjectorDeviceID = 130, 
    UnknownProjectorDeviceID = 131, 
    CantOpenProjectorFwFile = 132,
    UnexpectedChecksum = 133,
    CantEraseProjectorSector = 134,
    CantValidatePatternSequence = 135,
    BadProjectorHWStatus = 136,
    BadProjectorSystemStatus = 137,
    BadProjectorMainStatus = 138,
    VideoModeError = 139,
    DrmCantOpenDevice = 140,
    DrmCantGetResources = 141,
    DrmConnectorIndexOutOfBounds = 142,
    DrmCantRetrieveConnector = 143,
    DrmModeNotAvailable = 144,
    DrmCantRetrieveEncoder = 145,
    DrmNoDumbBufferSupport = 146,
    DrmCantCreateDumbBuffer = 147,
    DrmCantCreateFrameBuffer = 148,
    DrmConnectorNotConnected = 149,
    DrmCantSetCrtc = 150,
    DrmCantPrepareDumbBuffer = 151,
    DrmCantMapDumbBuffer = 152,
    DrmCantGetCapability = 153,
    CantOpenMemoryDevice = 154,
    CantMapPriorityRegister = 155,
    CantUnMapPriorityRegister = 156,
    BadPerLayerSettings = 157,
    GpioOutput = 158,

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
            messages[GpioOutput] = "Unable to open output for %d";  
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
            messages[LoadImageError] = "Error loading image: %s";
            messages[NoImageForLayer] = "No image for layer %d";
            messages[CantShowImage] = "Can't show image for layer %d";
            messages[CantShowBlack] = "Can't clear the screen to black: %s";
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
            messages[IllegalStateForUISubState] = "Printer must be in Home or DoorOpen state to change its UI sub-state, was in state %s";
            messages[UnknownPrintEngineState] = "Unknown print engine state: %d";
            messages[UnknownUISubState] = "Unknown UI sub-state: %d";
            messages[FrontPanelNotReady] = "Timeout awaiting front panel ready";
            messages[LongFrontPanelString] = "String too long (%d) to be shown on front panel";
            messages[UnknownPrintEngineSubState] = "Unknown print engine UI sub-state: %d";
            messages[CantReadRegistrationInfo] = "Can't read the registration info file from file: %s";
            messages[CantLoadSettingsFile] = "Can't load settings file: %s";
            messages[OverHeated] = "Printer temperature (%sC) is too high";
            messages[CantOpenThermometer] = "Can't find temperature sensor file";
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
            messages[IPThreadAlreadyRunning] = "Image processing thread is already running",
            messages[CantStartIPThread] = "Unable to start the image processing thread",
            messages[CantJoinIPThread] = "Unable to join the image processing thread",                  
            messages[ImageProcessing] = "Error processing image: %s",
            messages[CantShowWhite] = "Can't clear the screen to white: %s";
            messages[I2cReadReadWhenReady] = "Read error in I2C_Device::ReadWhenReady";
            messages[I2cDeviceNotReady] = "I2C_Device not ready for reading";
            messages[ProjectorGammaError] = "Could not disable projector's gamma correction";
            messages[PatternModeError] = "Could not set pattern mode";
            messages[VideoModeError] = "Could not set video mode";
            messages[ProjectorUpgradeError] = "Could not upgrade projector firmware";
            messages[CantReadProjectorFwVersion] = "Could not read projector firmware version";
            messages[CantReadProjectorMfrID] = "Could not read projector manufacturer ID";
            messages[UnknownProjectorMfrID] = "Unknown projector manufacturer ID: 0x%X";
            messages[CantReadProjectorDeviceID] = "Could not read projector device ID";
            messages[UnknownProjectorDeviceID] = "Unknown projector device ID: 0x%X";
            messages[CantOpenProjectorFwFile] = "Could not open projector firmware file: %s";
            messages[UnexpectedChecksum] = "Unexpected checksum: 0x%X";
            messages[CantEraseProjectorSector] = "Could not erase projector sector at 0x%X";
            messages[CantValidatePatternSequence] = "Could not validate pattern sequence";
            messages[BadProjectorHWStatus] = "Invalid projector hardware status: 0x%X";
            messages[BadProjectorSystemStatus] = "Invalid projector system status: 0x%X";
            messages[BadProjectorMainStatus] = "Invalid projector main status: 0x%X";
            messages[DrmCantOpenDevice] = "Could not open DRM graphics device";
            messages[DrmCantGetResources] = "Could not retrieve DRM resources";
            messages[DrmConnectorIndexOutOfBounds] = "Specified DRM connector index out of bounds";
            messages[DrmCantRetrieveConnector] = "Could not retrieve DRM connector";
            messages[DrmModeNotAvailable] = "Requested DRM mode not available";
            messages[DrmCantRetrieveEncoder] = "Could not retrieve DRM encoder";
            messages[DrmNoDumbBufferSupport] = "DRM device does not support dumb buffers";
            messages[DrmCantCreateDumbBuffer] = "Could not create DRM dumb buffer";
            messages[DrmCantCreateFrameBuffer] = "Could not create DRM frame buffer";
            messages[DrmConnectorNotConnected] = "DRM connector not connected to display";
            messages[DrmCantSetCrtc] = "Could not set DRM CRTC mode";
            messages[DrmCantPrepareDumbBuffer] = "Could not prepare DRM dumb buffer for mapping";
            messages[DrmCantMapDumbBuffer] = "Could not memory map DRM dumb buffer";
            messages[DrmCantGetCapability] = "Could not get DRM device capability";
            messages[CantOpenMemoryDevice] = "Could not open memory device to prevent video flicker";
            messages[CantMapPriorityRegister] = "Could not map priority register to prevent video flicker";
            messages[CantUnMapPriorityRegister] = "Could not un-map priority register to prevent video flicker";
            messages[BadPerLayerSettings] = "Invalid per-layer settings file";
                    
            messages[UnknownErrorCode] = "Unknown error code: %d";
            initialized = true;
        }

        if (errorCode < Success ||  errorCode >= MaxErrorCode)
        {
            // don't use Logger here, to avoid recursion
            char buf[255];
            sprintf(buf, messages[UnknownErrorCode], errorCode);
            syslog(LOG_WARNING, buf);
            std::cerr << buf << std::endl;
            return "";                                                              
        }
        return messages[errorCode];    
    }    
    
    // Get a collection of short error messages for display in multiple lines
    // on the front panel.
    static std::vector<const char*> GetShortMessages(ErrorCode errorCode)
    {
        static bool initialized = false;
        static std::vector<const char*> messages[MaxErrorCode];
        if (!initialized)
        {
            // initialize the array of short error messages
            for (ErrorCode ec = Success; ec < MaxErrorCode; 
                                         ec = (ErrorCode)(((int)ec) + 1))
            {
                // error codes without short messages are initialized empty
                messages[ec] = {""};
            }
                        
            messages[GpioInput] =                   {"Door sensor"};
            messages[MotorTimeoutTimer] =           {"Can't set motor",
                                                     "timeout timer."};
            messages[ExposureTimer] =               {"Exposure timer"};
            messages[MotorTimeoutError] =           {"Motor timeout"};
            messages[MotorError] =                  {"Motion control"};
            messages[MotorSettingError] =           {"Motion control"};
            messages[UnexpectedMotionEnd] =         {"Motion control"};
            messages[RemainingExposure] =           {"Can't read remaining",
                                                     "exposure time."};
            messages[NoImageForLayer] =             {"Missing image for",
                                                     "print layer."};
            messages[CantShowImage] =               {"Can't project print",
                                                     "layer image."};
            messages[CantShowBlack] =               {"Can't set projector",
                                                     "to black."};
            messages[CantShowWhite] =               {"Can't set projector",
                                                     "to white"};
            messages[CantGetSetting] =              {"Can't read setting"};
            messages[CantLoadSettings] =            {"Can't load",
                                                     "settings file."};
            messages[CantLoadSettingsFile] =        {"Can't load settings",
                                                     "from file."};
            messages[CantRestoreSettings] =         {"Can't restore",
                                                     "settings file."};
            messages[CantRestorePrintSettings] =    {"Can't restore",
                                                     "print settings."};
            messages[CantSaveSettings] =            {"Can't save",
                                                     "settings file."};
            messages[CantReadSettingsString] =      {"Can't read",
                                                     "settings text."};
            messages[NoDefaultSetting] =            {"Default setting",
                                                     "value is missing."};
            messages[UnknownSetting] =              {"Unknown setting",
                                                     "You may need a",
                                                     "firmware upgrade."};
            messages[SettingOutOfRange] =           {"Setting is",
                                                     "out of range."};
            messages[WrongTypeForSetting] =         {"Invalid setting",
                                                     "found."};
            messages[OverHeated] =                  {"Ember is overheated.",
                                                     "Turn of and allow",
                                                     "to cool down."};
            messages[NoValidPrintDataAvailable] =   {"Invalid print data",
                                                     "entered. Upload new",
                                                     "print data."};
            messages[ImageProcessing] =             {"Can't process print",
                                                     "layer image."};
            messages[ProjectorGammaError] =         {"Projector needs",
                                                     "to be restarted.",
                                                     "Cycle power to fix."};
            messages[PatternModeError] =            {"Could not put",
                                                     "projector into",
                                                     "pattern mode."}; 
            messages[VideoModeError] =              {"Could not put",
                                                     "projector into",
                                                     "video mode."};
            messages[ProjectorUpgradeError] =       {"Could not",
                                                     "upgrade projector."};
            messages[BadPerLayerSettings] =         {"Invalid per-layer",
                                                     "settings file."};
                     
            initialized = true;
        }

        if (errorCode < Success || errorCode >= MaxErrorCode)
        {
            // this error will already have been logged, when attempting
            // to access the corresponding long error message
            return {""};                                                              
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

