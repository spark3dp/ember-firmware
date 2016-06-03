//  File:   Projector.cpp
//  Encapsulates the functionality of the printer's projector
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015, 2016 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//  Jason Lefley
//
//  The Projector methods EnterProgramMode, UpgradeFirmware, ReadChecksum, 
//  ProgramFlash, EraseSector, and the flashSectorAddress array were  
//  derived from "DLPC350 I2C FW load example code", 
//  Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
//  modified by Autodesk in 2016
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

#include <iostream>

#include "Projector.h"
#include "I_I2C_Device.h"
#include "Hardware.h"
#include "Logger.h"
#include "MessageStrings.h"
#include "IFrameBuffer.h"
#include "Settings.h"
#include "utils.h"
#include "Filenames.h"
#include "HardwareFactory.h"

// delay times for projector control, expressed in microseconds
constexpr unsigned int DELAY_10_Ms  =  10000;
constexpr unsigned int DELAY_100_Ms = 100000;

Projector::Projector(const I_I2C_Device& i2cDevice) :
_i2cDevice(i2cDevice),
_supportsPatternMode(false),
_totalProgramBytes(0L),
_programBytesWritten(0L),
_runningChecksum(0L),
_programmingComplete(false),
_pFirmwareFile(NULL)
{
    // see if we have an I2C connection to the projector
    _canControlViaI2C = (I2CRead(PROJECTOR_HW_STATUS_REG) != ERROR_STATUS);

    if (!_canControlViaI2C)
        Logger::LogMessage(LOG_INFO, LOG_NO_PROJECTOR_I2C);
    else
    {
        // read projector Firmware Version
        unsigned char buf[16];
        if (!_i2cDevice.ReadWhenReady(PROJECTOR_FW_VERSION_REG, buf, 16, 
                                                       PROJECTOR_READY_STATUS))
        {
            Logger::LogError(LOG_ERR, errno, CantReadProjectorFwVersion);
        } 
        else
        {
            // currently there is only one version of projector firmware that
            // supports changing to/from pattern mode via I2C
            _supportsPatternMode = buf[3] == CURRENT_PROJECTOR_FW_MAJ_VERSION && 
                                   buf[2] == CURRENT_PROJECTOR_FW_MIN_VERSION;
        }
    }

    TurnLEDOff();
}

Projector::~Projector() 
{
    // don't throw exceptions from destructor
    try
    {
        TurnLEDOff();
        if(_pFirmwareFile != NULL)
            fclose(_pFirmwareFile);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

// Sets the image for display but does not actually draw it to the screen.
void Projector::SetImage(Magick::Image& image)
{
    if (_pFrameBuffer)
    {
        _pFrameBuffer->Blit(image);
    }
}

// Display the currently held image.
void Projector::ShowCurrentImage()
{
    if (_pFrameBuffer)
    {
        _pFrameBuffer->Swap();
    }
    TurnLEDOn();
}

// Display an all black image.
void Projector::ShowBlack()
{
    TurnLEDOff();
    if (_pFrameBuffer)
    {
        _pFrameBuffer->Fill(0x00);
    }
}

// Display an all white image.
void Projector::ShowWhite()
{
    if (_pFrameBuffer)
    {
        _pFrameBuffer->Fill(0xFF);
    }
    TurnLEDOn();

}

// Turn the projector's LED(s) off.
void Projector::TurnLEDOff()
{
    if (!_canControlViaI2C)
        return;
 
    I2CWrite(PROJECTOR_LED_ENABLE_REG, PROJECTOR_DISABLE_LEDS);
}

// Set the projector's LED(s) current and turn them on. Set the current every
// time to prevent having to restart the system to observe the effects of
// changing the LED current setting.
void Projector::TurnLEDOn()
{
    if (!_canControlViaI2C)
        return;
 
    // set the LED current, if we have a valid setting value for it
    int current = PrinterSettings::Instance().GetInt(PROJECTOR_LED_CURRENT);
    
    if (current > 0)
    {
        // Set the PWM polarity.
        // Though the PRO DLPC350 Programmer’s Guide says to set this after 
        // setting the LED currents, it appears to need to be set first.
        // Also, the Programmer’s Guide seems to have the 
        // polarity backwards.
        I2CWrite(PROJECTOR_LED_PWM_POLARITY_REG, PROJECTOR_PWM_POLARITY_NORMAL);
        
        unsigned char c = static_cast<unsigned char>(current);

        // use the same value for all three LEDs
        unsigned char buf[3] = {c, c, c};

        I2CWrite(PROJECTOR_LED_CURRENT_REG, buf, 3);
    }

    I2CWrite(PROJECTOR_LED_ENABLE_REG, PROJECTOR_ENABLE_LEDS);
}

constexpr int MAX_DISABLE_GAMMA_ATTEMPTS = 5;

// Attempt to disable the projector's gamma correction, to provide linear output.  
// Returns false if it cannot be disabled.
bool Projector::DisableGamma()
{
    if(!_canControlViaI2C)
        return true;  
        
    for(int i = 0; i < MAX_DISABLE_GAMMA_ATTEMPTS; i++)
    {
        std::cout << DISABLING_GAMMA_MSG << std::endl;

        // send the I2C command to disable gamma
        I2CWrite(PROJECTOR_GAMMA, PROJECTOR_GAMMA_DISABLE);
                
        unsigned char mainStatus = I2CRead(PROJECTOR_MAIN_STATUS_REG);
        if(mainStatus != ERROR_STATUS && 
           (mainStatus & PROJECTOR_GAMMA_ENABLED) == 0)
            return true;
    }
    return false;
}

// Attempt to put the projector into pattern mode.  
// Returns false if pattern mode cannot be set.
bool Projector::SetPatternMode()
{   
    if(!_canControlViaI2C || !_supportsPatternMode)
        return false;   // can't set pattern mode
       
    if (!SetVideoResolution(PATTERN_MODE_WIDTH, PATTERN_MODE_HEIGHT))
        return false;   // can't set required video resolution

    // stop any sequence already in progress, if any 
    // (though there should never be one, since we're in video mode)
    I2CWrite(PROJECTOR_PATTERN_START_REG, PROJECTOR_STOP_PATTERN_SEQ);
    usleep(DELAY_100_Ms);
    
    // step numbers below are from sec 4.1 of PRO DLPC350 Programmer’s Guide
    // 1. set pattern mode   
    I2CWrite(PROJECTOR_DISPLAY_MODE_REG, PROJECTOR_PATTERN_MODE);
    usleep(DELAY_100_Ms);
    
    // 2. select video as pattern input source
    I2CWrite(PROJECTOR_PATTERN_SOURCE_REG, 0);
    usleep(DELAY_100_Ms);
    
    // 3. set pattern LUT control
    unsigned char lut[4] = {0,   // one entry
                            1,   // repeat
                            0,   // one pattern
                            0}; // irrelevant

    I2CWrite(PROJECTOR_PATTERN_LUT_CTL_REG, lut, 4);
    usleep(DELAY_100_Ms);
    
    // 4. set trigger mode 0
    I2CWrite(PROJECTOR_PATTERN_TRIGGER_REG, 0);
    usleep(DELAY_100_Ms);
            
    // 5. set pattern exposure time and frame period
    unsigned char times[8] = {0x1B, 0x41, 0, 0,  // 0x411B = 16667 microseconds
                              0x1B, 0x41, 0, 0}; 
    I2CWrite(PROJECTOR_PATTERN_TIMES_REG, times, 8);
    usleep(DELAY_100_Ms);
    
    // (step 6 not needed)
    // 7.a. open LUT mailbox
    I2CWrite(PROJECTOR_PATTERN_LUT_ACC_REG, 2);
    usleep(DELAY_100_Ms);
    
    // 7.b. set mailbox offset
    I2CWrite(PROJECTOR_PATTERN_LUT_OFFSET_REG, 0);
    usleep(DELAY_100_Ms);
    
    // 7.c. fill pattern data
    unsigned char data[3] = {0 | (0 << 2),  // internal trigger, pattern 0 
                             7 | (4 << 4),  // 7-bit, Blue LED   
                             0};            // no options needed here     
    I2CWrite(PROJECTOR_PATTERN_LUT_DATA_REG, data, 3);
    usleep(DELAY_100_Ms);
    
    // 7.d. close LUT mailbox
    I2CWrite(PROJECTOR_PATTERN_LUT_ACC_REG, 0);
    usleep(DELAY_100_Ms);
    
    // 8. validate the commands
    I2CWrite(PROJECTOR_VALIDATE_REG, 0);
    // wait for validation to complete
    usleep(DELAY_100_Ms);  
    unsigned char status = I2CRead(PROJECTOR_VALIDATE_REG);
    if(status == ERROR_STATUS || (status & PROJECTOR_VALID_DATA) != 0)
    {
        Logger::LogError(LOG_ERR, errno, CantValidatePatternSequence);
        return false;
    }
    
    usleep(DELAY_100_Ms);
    
    // 9. read status
    if(!PollStatus())
        return false;   // 10. handle error
    usleep(DELAY_100_Ms);
     
    // 11. start pattern mode
    I2CWrite(PROJECTOR_PATTERN_START_REG, PROJECTOR_START_PATTERN_SEQ);
    
    return true;
}
    
// Poll system status as required after sending commands to switch between
// video and pattern modes.  Returns false if an error is detected.
bool Projector::PollStatus()
{
    usleep(DELAY_100_Ms);
    unsigned char status = I2CRead(PROJECTOR_HW_STATUS_REG);
    // std::cout << "HW status = " << (int)status << std::endl;
    if(status == ERROR_STATUS || (status & PROJECTOR_INIT_ERROR) == 0) 
    {
        Logger::LogError(LOG_ERR, errno, BadProjectorHWStatus, status);
        return false;  
    }
    
    usleep(DELAY_100_Ms);
    status = I2CRead(PROJECTOR_SYSTEM_STATUS_REG);
    // std::cout << "system status = " << (int)status << std::endl;
    if(status == ERROR_STATUS || (status & PROJECTOR_SYSTEM_MEMORY_FLAG) == 0)
    {
        Logger::LogError(LOG_ERR, errno, BadProjectorSystemStatus, status);
        return false;  
    }   

    usleep(DELAY_100_Ms);
    status = I2CRead(PROJECTOR_MAIN_STATUS_REG);
    // std::cout << "main status = " << (int)status << std::endl;
    // the Frame Buffer Swap Flag may be set here, 
    // when going from pattern to video mode, but that doesn't seem to
    // indicate an error
    if(status == ERROR_STATUS) 
    {
        Logger::LogError(LOG_ERR, errno, BadProjectorMainStatus, status);
        return false;  
    }
    else    
        return true; 
}

// Attempt to put the projector into video mode.  
// Returns false if video mode cannot be set.
bool Projector::SetVideoMode()
{    
    if(_canControlViaI2C)
    {
        // exit pattern mode
        I2CWrite(PROJECTOR_DISPLAY_MODE_REG, PROJECTOR_VIDEO_MODE);
        usleep(DELAY_100_Ms);

        if(!PollStatus())
            return false;
    }
    
    usleep(DELAY_100_Ms);
    
    if (!SetVideoResolution(VIDEO_MODE_WIDTH, VIDEO_MODE_HEIGHT))
        return false;
    
    if(!DisableGamma())
        Logger::LogError(LOG_ERR, errno, ProjectorGammaError);
    return true;
}

// Wrapper that sets msb of register address, as required by the projector.
bool Projector::I2CWrite(unsigned char registerAddress, unsigned char data)
{
    return _i2cDevice.Write(registerAddress | PROJECTOR_WRITE_BIT, data);
}

// Wrapper that sets msb of register address, as required by the projector.
bool Projector::I2CWrite(unsigned char registerAddress, 
                         const unsigned char* data, int length)
{
    return _i2cDevice.Write(registerAddress | PROJECTOR_WRITE_BIT, data, length);
}

// Wrapper that reads when ready, as required by the projector.
unsigned char Projector::I2CRead(unsigned char registerAddress)
{
    return _i2cDevice.ReadWhenReady(registerAddress, PROJECTOR_READY_STATUS);
}

// flash sectors to be erased
// (from the LightCrafter 4500 GUI application FlashDeviceParameters.txt) 
unsigned flashSectorAddress[71] = 
    { 0x000000, 0x008000, 0x010000, 0x018000, 0x020000, 0x030000, 0x040000, 
      0x060000, 0x080000, 0x0A0000, 0x0C0000, 0x0E0000, 0x100000, 0x120000, 
      0x140000, 0x160000, 0x180000, 0x1A0000, 0x1C0000, 0x1E0000, 0x200000, 
      0x220000, 0x240000, 0x260000, 0x280000, 0x2A0000, 0x2C0000, 0x2E0000, 
      0x300000, 0x320000, 0x340000, 0x360000, 0x380000, 0x3A0000, 0x3C0000, 
      0x3E0000, 0x400000, 0x420000, 0x440000, 0x460000, 0x480000, 0x4A0000, 
      0x4C0000, 0x4E0000, 0x500000, 0x520000, 0x540000, 0x560000, 0x580000, 
      0x5A0000, 0x5C0000, 0x5E0000, 0x600000, 0x620000, 0x640000, 0x660000, 
      0x680000, 0x6A0000, 0x6C0000, 0x6E0000, 0x700000, 0x720000, 0x740000, 
      0x760000, 0x780000, 0x7A0000, 0x7C0000, 0x7E0000, 0x7E8000, 0x7F0000, 
      0x7F8000};

// Puts the projector into Program Mode (if it was not already in that mode),
// or takes it out of that mode (if it was already in that mode)
bool Projector::EnterProgramMode(bool enter)
{
    unsigned char cmd;
    bool retVal = false;
    
    if (enter)
    {
        cmd = PROJECTOR_ENTER_PROGRAM_MODE;  
        retVal= I2CWrite(PROJECTOR_PROGRAM_MODE_REG, &cmd, 1);
        // here we need to wait 5 s for for the projector controller to jump 
        // to the boot-loader program
        _totalProgramBytes = 0L; 
        _programBytesWritten = 0L;
        _runningChecksum = 0L; 
        _programmingComplete = false;
    }
    else
    {
        cmd = PROJECTOR_LEAVE_PROGRAM_MODE;  
        // don't set high bit of register when in Program Mode
        retVal = _i2cDevice.Write(PROJECTOR_PROGRAM_MODE_REG, &cmd, 1);
        
        // this doesn't seem to work to get the projector out of Program Mode, 
        // whether we set the high bit or not
    }
    return retVal;
}

// Get the percent completion of programming, based on the total number of bytes
// to be written.
double Projector::GetUpgradeProgress()
{
    if(_totalProgramBytes == 0L)
        return 0.0;
    else
        return  _programBytesWritten / (double) _totalProgramBytes;   
}


// This is called repeatedly, until programming is complete.  It assumes the
// projector is already in Program Mode. 
bool Projector::UpgradeFirmware()
{     
    unsigned char wr_buf[256];
    unsigned char rd_buf[256];
    
    if(_totalProgramBytes == 0L)  // first stage
    {
        // open the firmware binary file 
        _pFirmwareFile = fopen(PROJECTOR_FW_FILE, "rb");
        if (_pFirmwareFile == NULL)
        {
            Logger::LogError(LOG_ERR, errno, CantOpenProjectorFwFile, 
                                                            PROJECTOR_FW_FILE);
            return false;
        }

        // request the Manufacturer's ID
        unsigned char cmd = PROJECTOR_GET_MFR_ID; 
        if(!I2CWriteAndRead(PROJECTOR_READ_CONTROL_REG, &cmd, 1, &rd_buf[0], 10))
        {
            Logger::LogError(LOG_ERR, errno, CantReadProjectorMfrID);
            return false;
        }    
        usleep(DELAY_10_Ms);
        int mfrID = 
               (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
        if (mfrID != SUPPORTED_PROJECTOR_MFR_ID) 
        {
            Logger::LogError(LOG_ERR, errno, UnknownProjectorMfrID, mfrID);
            return false;
        }

        // request the Device ID
        cmd = PROJECTOR_GET_DEVICE_ID; 
        if (!I2CWriteAndRead(PROJECTOR_READ_CONTROL_REG, &cmd, 1, rd_buf, 10))
        {
            Logger::LogError(LOG_ERR, errno, CantReadProjectorDeviceID);
            return false;
        }
        usleep(DELAY_10_Ms);
        int deviceID = 
               (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
        if (deviceID != SUPPORTED_PROJECTOR_DEVICE_ID)
        {
            Logger::LogError(LOG_ERR, errno, UnknownProjectorDeviceID, deviceID);
            return false;
        }

        // find the size of the binary file
        fseek(_pFirmwareFile, 0L, SEEK_END);
        _totalProgramBytes = ftell(_pFirmwareFile);

        // erase sectors  
        int i = 0;
        while(_totalProgramBytes > flashSectorAddress[i])
        {
            // but skip the bootloader area
            if (flashSectorAddress[i] >= APP_START_ADDR)
            {
                int tries = 0;
                while(!EraseSector(flashSectorAddress[i]))
                {
                    // try up to 10 times to erase each sector
                    if(++tries > 10)
                    {
                        Logger::LogError(LOG_ERR, errno, 
                               CantEraseProjectorSector, flashSectorAddress[i]);
                        return false;
                    }
                }
            }
            i++;
        }

        // set file pointer to beginning
        fseek(_pFirmwareFile, 0L, SEEK_SET);
        // point to the beginning of the main application,
        // skipping the 128k boot loader area
        fseek(_pFirmwareFile, APP_START_ADDR, SEEK_SET);

        // likewise, set the projector's start address to that position 
        wr_buf[0] =  APP_START_ADDR & 0xFF;
        wr_buf[1] = (APP_START_ADDR & 0xFF00)   >> 8;
        wr_buf[2] = (APP_START_ADDR & 0xFF0000) >> 16;
        wr_buf[3] = 0x00;
        _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG, wr_buf, 4);
        usleep(DELAY_10_Ms);

        // number of bytes to be written is less the 128k bootloader  
        _totalProgramBytes -= APP_START_ADDR; 

        // set download data size 
        wr_buf[0] =  _totalProgramBytes & 0xFF;
        wr_buf[1] = (_totalProgramBytes & 0xFF00)   >> 8;
        wr_buf[2] = (_totalProgramBytes & 0xFF0000) >> 16;
        wr_buf[3] = 0x00;
        _i2cDevice.Write(PROJECTOR_DATA_SIZE_REG, wr_buf, 4);

        _runningChecksum = 0x00;
        _programBytesWritten = 0;
        return true;   // first stage complete
    }
    else if (_programBytesWritten < _totalProgramBytes) // middle stage
    {
        // loop here so that we only occasionally incur the overhead of 
        // updating progress, since the LED ring doesn't provide 
        // many progress increments anyway
        for(int i = 0; i < 150; i++)
        {
            // check if this is the last transaction
            if ((_totalProgramBytes - _programBytesWritten) <= 256)
            {
                // read the remaining data from the file
                fread(rd_buf, sizeof(unsigned char), 
                                    _totalProgramBytes - _programBytesWritten, 
                                    _pFirmwareFile);
                // write the remaining number of bytes
                ProgramFlash(rd_buf, _totalProgramBytes - _programBytesWritten);
                // compute the checksum for last chunk 
                _runningChecksum += Checksum(rd_buf, 
                                     _totalProgramBytes - _programBytesWritten);
                _programBytesWritten = _totalProgramBytes;
                fclose(_pFirmwareFile);
                _pFirmwareFile = NULL;
                break;
            }
            else
            {                   
                // read from the binary file, 256 bytes at a time
                fread(rd_buf, sizeof(unsigned char), 256, _pFirmwareFile);
                // program the flash 256 bytes at a time
                ProgramFlash(rd_buf, 256); 
                _programBytesWritten += 256;
                _runningChecksum += Checksum(rd_buf, 256);
            }
        }
        return true;
    }
    else    // final stage
    {
        // get the checksum calculated by the projector
        unsigned long int actualChecksum = 
                               ReadChecksum(APP_START_ADDR, _totalProgramBytes);
        if (actualChecksum != _runningChecksum)
        {
            Logger::LogError(LOG_ERR, errno, UnexpectedChecksum, actualChecksum);
            return false;
        } 
        else
            _programmingComplete = true;
    }
    return true;  
}


// Read the checksum for the given area in the projector's flash memory
unsigned long int Projector::ReadChecksum(unsigned long int startAddress, 
                                          unsigned long int numBytes)
{
    int timeout = 0;
    unsigned char wr_buf[256];
    unsigned char rd_buf[256];

    //Set start address
    wr_buf[0] =  startAddress & 0xFF;
    wr_buf[1] = (startAddress & 0xFF00)   >> 8;
    wr_buf[2] = (startAddress & 0xFF0000) >> 16;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG, wr_buf, 4);
    usleep(DELAY_10_Ms);
    //Set number of bytes for checksum calculation 
    wr_buf[0] =  numBytes & 0xFF;
    wr_buf[1] = (numBytes & 0xFF00)   >> 8;
    wr_buf[2] = (numBytes & 0xFF0000) >> 16;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_DATA_SIZE_REG, wr_buf, 4);
    usleep(DELAY_10_Ms);

    // issue checksum compute command
    wr_buf[0] = 0x01;
    _i2cDevice.Write(PROJECTOR_CALCULATE_CHECKSUM_REG, wr_buf, 1);
    usleep(DELAY_100_Ms);

    // poll for checksum computation completion 
    while(1)
    {
        unsigned char status = _i2cDevice.Read(PROJECTOR_READ_CONTROL_REG);
        usleep(DELAY_10_Ms);

        if(status & FLASH_BUSY_STATUS) 
        {
            // flash access busy
            timeout++;
            // wait up to 10 seconds 
            if(timeout >= 1000)
                return -1;
        }
        else
        {
            // checksum calculation complete, read its value
            wr_buf[0] = PROJECTOR_GET_CHECKSUM;
            I2CWriteAndRead(PROJECTOR_READ_CONTROL_REG, wr_buf, 1, rd_buf, 10);
            usleep(DELAY_10_Ms);
            //BYTE0 BYTE1 BYTE2 BYTE3 BYTE4 BYTE5 []BYTE6 BYTE7 BYTE8 BYTE9]  Checksum [xx xx xx xx] [LSB .. .. MSB] BYTE6 - BYTE9 return checksum
            //checksum = BYTE9<<24 | BYTE8<<16 | BYTE7<<8 | BYTE6
            return (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
        }
    }
}

// Program flash with data provided from the given buffer
void Projector::ProgramFlash(unsigned char *buf, unsigned int numBytes)
{
    _i2cDevice.Write(PROJECTOR_DOWNLOAD_DATA_REG, buf, numBytes);
    usleep(DELAY_10_Ms);
}

/* Function: Erase the flash sector depending upon the user provided sector address */
bool Projector::EraseSector(unsigned long sectorAddress)
{
    int timeout = 0;
    unsigned char wr_buf[4];
    unsigned char rd_buf[4];

    wr_buf[0] =  sectorAddress & 0xFF;
    wr_buf[1] = (sectorAddress & 0xFF00)   >> 8;
    wr_buf[2] = (sectorAddress & 0xFF0000) >> 16;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG, wr_buf, 4);
    usleep(DELAY_10_Ms);

    wr_buf[0] = 0x01;
    _i2cDevice.Write(PROJECTOR_ERASE_SECTOR_REG, wr_buf, 1);
    usleep(DELAY_10_Ms);

    while(1)
    {
        unsigned char status = _i2cDevice.Read(PROJECTOR_READ_CONTROL_REG);

        usleep(DELAY_10_Ms);

        if(status & FLASH_BUSY_STATUS) 
        {
            // flash access busy
            timeout++;
            // wait up to 1 second 
            if(timeout >= 100)
                return false;
        }
        else
        {
            // erased a sector
            return true;
        }
    }
}

bool Projector::I2CWriteAndRead(unsigned char regAddress, 
                                unsigned char *writeBuf, 
                                unsigned numBytesToWrite, 
                                unsigned char *readBuf, 
                                unsigned numBytesToRead)
{
    if(numBytesToWrite > 0)
    {
        // write without OR'ing 0x80 into register address
        _i2cDevice.Write(regAddress, writeBuf, numBytesToWrite);
    }

    // read without awaiting ready status
    return _i2cDevice.Read(regAddress, readBuf, numBytesToRead);
}

// Sets the video resolution by creating a new frame buffer with the specified
// width and height.
// Note: Clients must call this method before using any of the show methods.
bool Projector::SetVideoResolution(int width, int height)
{
    // Call reset() to delete the existing frame buffer.
    // The existing frame buffer must release its resources before this method
    // creates a new instance via CreateFrameBuffer().
    try
    {
        // de-select the current video source while creating the frame buffer
        if (_canControlViaI2C)
            I2CWrite(PROJECTOR_SOURCE_SELECT_REG, PROJECTOR_SOURCE_FPD_LINK);
        _pFrameBuffer.reset();
        _pFrameBuffer = std::move(HardwareFactory::CreateFrameBuffer(width, height));
        if (_canControlViaI2C)
        {
            // wait for the video to stabilize
            sleep(1);            
            // re-select the actual video source
            I2CWrite(PROJECTOR_SOURCE_SELECT_REG, PROJECTOR_SOURCE_PARALLEL_24);
            // the source switch turns on the LED, so turn it off again
            TurnLEDOff();
            // wait for the projector to sync up
            sleep(1);   
        }
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}