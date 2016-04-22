//  File:   Projector.cpp
//  Encapsulates the functionality of the printer's projector
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

Projector::Projector(const I_I2C_Device& i2cDevice, IFrameBuffer& frameBuffer) :
_i2cDevice(i2cDevice),
_frameBuffer(frameBuffer),
_canUpgrade(false)
{
    // see if we have an I2C connection to the projector
    _canControlViaI2C = (I2CRead(PROJECTOR_HW_STATUS_REG) != ERROR_STATUS);

    if (!_canControlViaI2C)
        Logger::LogMessage(LOG_INFO, LOG_NO_PROJECTOR_I2C);
    else
    {
        // read projector Firmware Version
        unsigned char buf[16];
        if(!_i2cDevice.ReadWhenReady(PROJECTOR_FW_VERSION_REG, buf, 16, 
                                                       PROJECTOR_READY_STATUS))
        {
            Logger::LogError(LOG_ERR, errno, CantReadProjectorFwVersion);
        }        
//        else if(buf[3] != CURRENT_PROJECTOR_FW_MAJ_VERSION || 
//                buf[2] != CURRENT_PROJECTOR_FW_MIN_VERSION)
else // for debug only!!!
        {
            // we have an I2C connection, and are not using the current 
            // version of projector firmware, so we are ripe for an upgrade
            _canUpgrade = true; 
        }
    }

    ShowBlack();
}

Projector::~Projector() 
{
    // don't throw exceptions from destructor
    try
    {
        ShowBlack();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

// Sets the image for display but does not actually draw it to the screen.
void Projector::SetImage(Magick::Image& image)
{
    _frameBuffer.Blit(image);
}

// Display the currently held image.
void Projector::ShowCurrentImage()
{
    _frameBuffer.Swap();
    TurnLEDOn();
}

// Display an all black image.
void Projector::ShowBlack()
{
    TurnLEDOff();
    _frameBuffer.Fill(0x00);
}

// Display an all white image.
void Projector::ShowWhite()
{
    _frameBuffer.Fill(0xFFFFFFFF);
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

constexpr int DELAY_100_MSEC = 100000; 
constexpr int MAX_VALIDATE_ATTEMPTS = 20;

// Attempt to put the projector into pattern mode.  
// Returns false if pattern mode cannot be set.
bool Projector::SetPatternMode()
{
 //   if(!_canControlViaI2C)
        return true;
    
//    // stop any sequence already in progress
//    _i2cDevice.Write(PROJECTOR_PATTERN_START_REG, 0);
//    DELAY_MSECS;
    
    // step numbers below are from sec 4.1 of PRO DLPC350 Programmer’s Guide
    // 1. set pattern mode
    usleep(DELAY_100_MSEC);
    I2CWrite(PROJECTOR_DISPLAY_MODE_REG, 1);
    usleep(DELAY_100_MSEC);
    
    // 2. select video as pattern input source
    I2CWrite(PROJECTOR_PATTERN_SOURCE_REG, 0);
    usleep(DELAY_100_MSEC);
    
    // 3. set pattern LUT control
    unsigned char lut[4] = {0,   // one entry
                            1,   // repeat
                            0,   // one pattern
                            0}; // irrelevant

    I2CWrite(PROJECTOR_PATTERN_LUT_CTL_REG, lut, 4);
    usleep(DELAY_100_MSEC);
    
    // 4. set trigger mode 0
    I2CWrite(PROJECTOR_PATTERN_TRIGGER_REG, 0);
    usleep(DELAY_100_MSEC);
            
    // 5. set pattern exposure time and frame period
    unsigned char times[8] = {0x1B, 0x41, 0, 0, // 0x411B = 16667 microseconds
                              0x1B, 0x41, 0, 0}; 
//    unsigned char times[8] = {0, 0, 0x41, 0x1B, // 0x411B = 16667 microseconds
//                              0, 0, 0x41, 0x1B}; 
    I2CWrite(PROJECTOR_PATTERN_TIMES_REG, times, 8);
    usleep(DELAY_100_MSEC);
    
    // (step 6 not needed)
    // 7.a. open LUT mailbox
    I2CWrite(PROJECTOR_PATTERN_LUT_ACC_REG, 2);
    usleep(DELAY_100_MSEC);
    
    // 7.b. set mailbox offset
    I2CWrite(PROJECTOR_PATTERN_LUT_OFFSET_REG, 0);
    usleep(DELAY_100_MSEC);
    
    // 7.c. fill pattern data
    unsigned char data[3] = {0 | (2 << 2),  // internal trigger, pattern 2 
                             8 | (1 << 4),  // 8-bit, RedLED   
                             0};            // no options needed here
//    unsigned char data[3] = {0x09,  // external trigger, pattern 2 
//                             0x18,  // 8-bit, Red LED   
//                             0x04}; // do buffer swap

    I2CWrite(PROJECTOR_PATTERN_LUT_DATA_REG, data, 3);
    usleep(DELAY_100_MSEC);
    
    // 7.d. close LUT mailbox
    I2CWrite(PROJECTOR_PATTERN_LUT_ACC_REG, 0);
    usleep(DELAY_100_MSEC);
    
    // 8. validate the commands
    I2CWrite(PROJECTOR_VALIDATE_REG | 0x80, 0);
    // wait for validation to complete
    bool succeeded = false;
    for(int i = 0; i < MAX_VALIDATE_ATTEMPTS; i++)
    {
        usleep(1000000);
        unsigned char status = I2CRead(PROJECTOR_VALIDATE_REG);
        if(status & 0x80)
        {
            std::cout << "validation not ready: " << i << std::endl;
            continue;
        }
        else if(status == ERROR_STATUS || (status & PROJECTOR_VALID_DATA) != 0)
            return false;
        else
        {
            //validation succeeded
            succeeded = true;
            break;
        }   
    }
    if(!succeeded)
        return false;
    
    usleep(DELAY_100_MSEC);
    
    // 9. read status
    if(!PollStatus())
        return false;   // 10. handle error
    usleep(DELAY_100_MSEC);
     
    // 11. start pattern mode
    // Though the PRO DLPC350 Programmer’s Guide says to use 0x10 here,
    // they must have meant b10, since only the two lsbs are used
    I2CWrite(PROJECTOR_PATTERN_START_REG, 2);
    
    return true;
}
    
// Poll system status as required after sending commands to switch between
// video and pattern modes.  Returns false if an error is detected.
bool Projector::PollStatus()
{
    usleep(DELAY_100_MSEC);
    unsigned char status = I2CRead(PROJECTOR_HW_STATUS_REG);
//    if(status == ERROR_STATUS || (status & PROJECTOR_INIT_ERROR) == 0 ||
//                                 (status & PROJECTOR_HW_ERROR) != 0)
//        return false;
    std::cout << "HW status = " << (int)status << std::endl;
    
    usleep(DELAY_100_MSEC);
    status = I2CRead(PROJECTOR_SYSTEM_STATUS_REG);
//    if(status == ERROR_STATUS || (status & 0x1) == 0)
//        return false;
    std::cout << "system status = " << (int)status << std::endl;

    usleep(DELAY_100_MSEC);
    status = I2CRead(PROJECTOR_MAIN_STATUS_REG);
    std::cout << "main status = " << (int)status << std::endl;

//    if(status == ERROR_STATUS || 
//       (status & PROJECTOR_SEQUENCER_RUN_FLAG) == 0 ||
//       (status & PROJECTOR_FB_SWAP_FLAG) != 0)
//        return false;
//    else
        return true; 
}

// Attempt to put the projector into video mode.  
// Returns false if video mode cannot be set.
bool Projector::SetVideoMode()
{
    if(!_canControlViaI2C)
        return true;
    
    usleep(DELAY_100_MSEC);
    I2CWrite(PROJECTOR_DISPLAY_MODE_REG, 0);
    usleep(DELAY_100_MSEC);

    if(!PollStatus())
        return false;

    // in case we started up in pattern mode, which doesn't support the gamma
    // correction commands, make sure it's disabled here
    usleep(DELAY_100_MSEC);
    DisableGamma();
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


// the following is based on TI's example code
//Register name 
#define REG_FLASH_DWLD              0x25
#define REG_FLASH_CHKSUM            0x26
#define REG_FLASH_ERASE             0x28

constexpr unsigned int APP_START_ADDR = 0x20000;

// Flash device ID and sector layout 
// Refer to the LightCrafter 4500 GUI application FlashDeviceParameters.txt 
// to understand the flash sector layout information
// with the help of Manufacturer's ID and Device ID get the sectors to be erased
//unsigned gflash_sec_add[71] = { 0x0, 0x8000, 0x10000, 0x18000, 0x20000, 0x30000, 0x40000, 0x60000, 0x80000, 0xA0000, 0xC0000, 0xE0000, 0x100000, 0x120000, 0x140000, 0x160000, 0x180000, 0x1A0000, 0x1C0000, 0x1E0000, 0x200000, 0x220000, 0x240000, 0x260000, 0x280000, 0x2A0000, 0x2C0000, 0x2E0000, 0x300000, 0x320000, 0x340000, 0x360000, 0x380000, 0x3A0000, 0x3C0000, 0x3E0000, 0x400000, 0x420000, 0x440000, 0x460000, 0x480000, 0x4A0000, 0x4C0000, 0x4E0000, 0x500000, 0x520000, 0x540000, 0x560000, 0x580000, 0x5A0000, 0x5C0000, 0x5E0000, 0x600000, 0x620000, 0x640000, 0x660000, 0x680000, 0x6A0000, 0x6C0000, 0x6E0000, 0x700000, 0x720000, 0x740000, 0x760000, 0x780000, 0x7A0000, 0x7C0000, 0x7E0000, 0x7E8000, 0x7F0000, 0x7F8000,};

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
        // here we need to wait 5 s
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

// assumes projector is already in Program Mode
bool Projector::UpgradeFirmware()
{    
    unsigned long int totalBytes; 
    unsigned long int bytesWritten;
    unsigned long int expectedChecksum; 
    unsigned long int actualChecksum; 
    unsigned char wr_buf[256];
    unsigned char rd_buf[256];
    unsigned int mfrID;
    unsigned int deviceID;
     
    // request the Manufacturer's ID
    unsigned char cmd = PROJECTOR_GET_MFR_ID; 
    if(!I2CRead(PROJECTOR_READ_CONTROL_REG, &cmd, 1, rd_buf, 10))
    {
        Logger::LogError(LOG_ERR, errno, CantReadProjectorMfrID);
        return false;
    }    
    usleep(10000);
    mfrID = (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
    if (mfrID != PROJECTOR_SUPPORTED_MFR_ID) 
    {
        Logger::LogError(LOG_ERR, errno, UnknownProjectorMfrID, mfrID);
        return false;
    }
    
    //Request the Device ID
    cmd = PROJECTOR_GET_DEVICE_ID; 
    if (!I2CRead(PROJECTOR_READ_CONTROL_REG, &cmd, 1, rd_buf, 10))
    {
        Logger::LogError(LOG_ERR, errno, CantReadProjectorDeviceID);
        return false;
    }
    usleep(10000);
    deviceID = (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
    if (deviceID != PROJECTOR_SUPPORTED_DEVICE_ID)
    {
        Logger::LogError(LOG_ERR, errno, UnknownProjectorDeviceID, deviceID);
        return false;
    }

    // open the firmware binary file 
    FILE* fp = fopen(GetFilePath(PROJECTOR_FW_FILE).c_str(), "rb");
    if (fp == NULL)
    {
        Logger::LogError(LOG_ERR, errno, CantOpenProjectorFwFile, 
                                                GetFilePath(PROJECTOR_FW_FILE));
        return false;
    }


    // find the size of the binary file
    fseek(fp, 0L, SEEK_END);
    totalBytes = ftell(fp);
    
    ///////////////////////////
    // Erase Sectors
    ///////////////////////////
    
    // (It's not clear that we need to do this at all, 
    // bypass for now while testing on systems already upgraded to 3.0)
 /*   
    int i = 0;
    while(total_num_bytes > gflash_sec_add[i])
    {
        //Note: Skip the bootloader area then you must skip first 128KB area
        if (gflash_sec_add[i] >= APP_START_ADDR)
        {
            Erase_Sector( gflash_sec_add[i]);
            printf("Erased sector %d\n", i);
        }
        i++;
    }
*/

    // program the projector's flash memory

    // set file pointer to beginning
    fseek(fp, 0L, SEEK_SET);
    // point to the beginning of the main application
    // skip the 128k boot loader area
    fseek(fp, APP_START_ADDR, SEEK_SET);

    // likewise set the start address to the beginning of the main application 
    wr_buf[0] =  APP_START_ADDR & 0xFF;
    wr_buf[1] = (APP_START_ADDR & 0xFF00) >> 8;
    wr_buf[2] = (APP_START_ADDR & 0xFF0000) >> 16;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG, &wr_buf[0], 4);
    usleep(10000);

    // number of bytes to be written is less the 128k for the bootloader area 
    totalBytes -= APP_START_ADDR; 

    // set download data size 
    wr_buf[0] = totalBytes & 0xFF;
    wr_buf[1] = ((totalBytes & 0xFF00) >> 8) & 0xFF;
    wr_buf[2] = ((totalBytes & 0xFF0000) >> 16) & 0xFF;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_DATA_SIZE_REG, &wr_buf[0], 4);
    usleep(10000);

    expectedChecksum = 0x00;
    bytesWritten = 0;

    // program the flash 256 bytes at a time
    while(bytesWritten < totalBytes)
    {

        // read from the binary file 256 bytes at a time
        fread(rd_buf, sizeof(unsigned char), 256, fp);

        bytesWritten += Program_Flash(rd_buf, 256); 

        expectedChecksum += Checksum(rd_buf, 256);

        // check if it is the last transaction
        if ((totalBytes - bytesWritten) <= 256)
        {
            // read the remaining data from the file
            fread(rd_buf, sizeof(unsigned char), totalBytes - bytesWritten, fp);
            // write the remaining number of bytes
            Program_Flash(&rd_buf[0], totalBytes - bytesWritten);
            // compute the checksum for last chunk 
            expectedChecksum += Checksum(rd_buf, totalBytes - bytesWritten);
            break;
        }
        
        // TODO: provide progress indication
        if (bytesWritten % 4096 == 0)
            printf("Programmed %d bytes\n", bytesWritten);
    }

    // get the checksum calculated by the projector
    actualChecksum = ReadChecksum(APP_START_ADDR, totalBytes);
    if (actualChecksum != expectedChecksum)
    {
        Logger::LogError(LOG_ERR, errno, UnexpectedChecksum, actualChecksum);
        return false;
    }    

    return true;  
}


// Reads the checksum for given area in the flash  
unsigned long int Projector::ReadChecksum(unsigned long int startAddress, unsigned long int numBytes)
{
    unsigned int flash_checksum = 0x00;
    int timeout = 0;
    unsigned char wr_buf[256];
    unsigned char rd_buf[256];

    //Set start address
    wr_buf[0] = startAddress & 0xFF;
    wr_buf[1] = ((startAddress & 0xFF00) >> 8) & 0xFF;
    wr_buf[2] = ((startAddress & 0xFF0000) >> 16) & 0xFF;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG,&wr_buf[0],4);
    usleep(10000);
    //Set number of bytes for checksum calculation 
    wr_buf[0] = numBytes & 0xFF;
    wr_buf[1] = ((numBytes & 0xFF00) >> 8) & 0xFF;
    wr_buf[2] = ((numBytes & 0xFF0000) >> 16) & 0xFF;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_DATA_SIZE_REG, wr_buf, 4);
    usleep(10000);

    // issue checksum compute command
    wr_buf[0] = 0x01;
    _i2cDevice.Write(REG_FLASH_CHKSUM, wr_buf, 1);
    usleep(100000);

    //Poll for checksum computation completion 
    while(1)
    {
        //Read one byte Status Byte
        //BIT_0 : System is ready 
        //BIT_1 : System is good 
        //BIT_2 : Reset 
        //BIT_3 : Flash access BUSY
        //BIT_4 : Mailbox download complete
        //BIT_5 : Command Error
        //BIT_6 : Reserved
        //BIT_7 : Program Mode
        wr_buf[0] = 0x00; //Request type
        I2CRead(PROJECTOR_READ_CONTROL_REG, wr_buf , 0, rd_buf, 1);
//        rd_buf[0] = _i2cDevice.Read(REG_READ_STATUS);
        usleep(10000);

        // If flash access busy
        if(rd_buf[0] & 0x08) 
        {
            timeout++;
            //Wait upto 10seconds 
            if(timeout>=1000)
                return -1;
        }
        else
        {
            //Okay checksum calculation complete 
            wr_buf[0] = 0x00;
    //        I2CRead(PROJECTOR_READ_CONTROL_REG,&wr_buf[0],0,&rd_buf[0],10);
            I2CRead(PROJECTOR_READ_CONTROL_REG,&wr_buf[0],1,&rd_buf[0],10);
            usleep(10000);
            //BYTE0 BYTE1 BYTE2 BYTE3 BYTE4 BYTE5 []BYTE6 BYTE7 BYTE8 BYTE9]  Checksum [xx xx xx xx] [LSB .. .. MSB] BYTE6 - BYTE9 return checksum
            //checksum = BYTE9<<24 | BYTE8<<16 | BYTE7<<8 | BYTE6
            flash_checksum = (rd_buf[9] << 24 | rd_buf[8] << 16 | rd_buf[7] << 8 | rd_buf[6]);
            return flash_checksum;
        }

    }
}

/* Program flash with data provided from *buf */
int Projector::Program_Flash(unsigned char *buf, unsigned int num_bytes)
{
    int timeout = 0;
    unsigned char wr_buf[4];
    unsigned char rd_buf[4];

    //Write into the flash 
    _i2cDevice.Write(REG_FLASH_DWLD,buf,num_bytes);
    usleep(10000);

    return num_bytes;
}

/* Function: Erase the flash sector depending upon the user provided sector address */
int Projector::Erase_Sector(unsigned long sector_address)
{
    int timeout = 0;
    unsigned char wr_buf[4];
    unsigned char rd_buf[4];

    wr_buf[0] = sector_address & 0xFF;
    wr_buf[1] = ((sector_address & 0xFF00) >> 8) & 0xFF;
    wr_buf[2] = ((sector_address & 0xFF0000) >> 16) & 0xFF;
    wr_buf[3] = 0x00;
    _i2cDevice.Write(PROJECTOR_START_ADDRESS_REG,&wr_buf[0],4);
    usleep(10000);

    wr_buf[0] = 0x01;
    _i2cDevice.Write(REG_FLASH_ERASE,&wr_buf[0],1);
    usleep(10000);

    while(1)
    {
        //Read one byte Status Byte
        //BIT_0 : System is ready 
        //BIT_1 : System is good 
        //BIT_2 : Reset 
        //BIT_3 : Flash access BUSY
        //BIT_4 : Mailbox download complete
        //BIT_5 : Command Error
        //BIT_6 : Reserved
        //BIT_7 : Program Mode
        wr_buf[0] = 0x00;
        I2CRead(PROJECTOR_READ_CONTROL_REG,&wr_buf[0],0,&rd_buf[0],1);
//        rd_buf[0] = _i2cDevice.Read(REG_READ_STATUS);

        usleep(10000);

        // If flash access busy
        if(rd_buf[0] & 0x08) 
        {
            timeout++;
            //Wait upto 1seconds 
            if(timeout>=100)
                return -1;
        }
        else
        {
            //Written 256 bytes into the buffer
            return 0;
        }

    }
}

bool Projector::I2CRead(unsigned char regAdd, unsigned char *wr_buf, 
                       unsigned num_bytes_write, unsigned char *rd_buf, 
                       unsigned num_bytes_read)
{
    if(num_bytes_write > 0)
    {
        // write without OR'ing 0x80 into register address
        _i2cDevice.Write(regAdd, wr_buf, num_bytes_write);
    }
    
    // may want some delay here
    // usleep(10000);

    // read without awaiting ready status
    return _i2cDevice.Read(regAdd, rd_buf, num_bytes_read);
}

