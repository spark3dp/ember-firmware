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

Projector::Projector(const I_I2C_Device& i2cDevice, IFrameBuffer& frameBuffer) :
_i2cDevice(i2cDevice),
_frameBuffer(frameBuffer)
{
    // see if we have an I2C connection to the projector
    _canControlViaI2C = (I2CRead(PROJECTOR_HW_STATUS_REG) != ERROR_STATUS);

    if (!_canControlViaI2C)
        Logger::LogMessage(LOG_INFO, LOG_NO_PROJECTOR_I2C);

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
constexpr int DELAY_1_SEC    = 1000000; 
constexpr int MAX_VALIDATE_ATTEMPTS = 20;

// Attempt to put the projector into pattern mode.  
// Returns false if pattern mode cannot be set.
bool Projector::SetPatternMode()
{
    if(!_canControlViaI2C)
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
