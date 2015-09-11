//  File: I2CInterface.cpp
//  I2C interface using AVR Two-Wire Interface (TWI) hardware
//
//  This file is part of the Ember Motor Controller firmware.
//
//  This file derives from Procyon AVRlib <http://www.procyonengineering.com/embedded/avr/avrlib/>.
//
//  Copyright 2002-2003 Pascal Stang
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <avr/interrupt.h>
#include <util/twi.h>

#include "I2CInterface.h"
#include "CommandBuffer.h"

#define I2C_ADDRESS 0x10

static MotorController_t *mcState;

// Initialize the I2C interface
void I2CInterface::Initialize(MotorController_t *mc)
{
    mcState = mc;

    // Enable TWI (two-wire interface)
    TWCR |= (1<<TWEN);

    // Enable TWI interrupt and slave address ACK
    TWCR |= (1<<TWIE) | (1<<TWEA);

    // Set local device address
    // Don't enable general call recognition
    TWAR = (I2C_ADDRESS<<1) | 0;
}

// I2C (TWI) interrupt service routine
ISR(TWI_vect)
{
    switch(TW_STATUS)
    {
        // Slave Receiver status codes
        case TW_SR_SLA_ACK:                 // 0x60: own SLA+W has been received, ACK has been returned
        case TW_SR_ARB_LOST_SLA_ACK:        // 0x68: own SLA+W has been received, ACK has been returned
        case TW_SR_GCALL_ACK:               // 0x70:     GCA+W has been received, ACK has been returned
        case TW_SR_ARB_LOST_GCALL_ACK:      // 0x78:     GCA+W has been received, ACK has been returned
            // SR->SLA_ACK
            // We are being addressed as slave for writing (data will be received from master)
            // Receive data byte and return ACK
            TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;
        case TW_SR_DATA_ACK:                // 0x80: data byte has been received, ACK has been returned
        case TW_SR_GCALL_DATA_ACK:          // 0x90: data byte has been received, ACK has been returned
            // SR->DATA_ACK

            // Store previously received data byte in buffer
            commandBuffer.AddCommandByte(TWDR);

            // Check receive buffer status
            if(commandBuffer.IsFull())
                // Receive data byte and return NACK
                TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN);
            else
                // Receive data byte and return ACK
                TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);

            break;
        case TW_SR_DATA_NACK:               // 0x88: data byte has been received, NACK has been returned
        case TW_SR_GCALL_DATA_NACK:         // 0x98: data byte has been received, NACK has been returned
            // SR->DATA_NACK
            // Receive data byte and return NACK
            TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN);
            break;
        case TW_SR_STOP:                    // 0xA0: STOP or REPEATED START has been received while addressed as slave
            // SR->SR_STOP
            // Switch to SR mode with SLA ACK
            TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            // Receive is complete
            break;

            // Slave Transmitter
        case TW_ST_SLA_ACK:                 // 0xA8: own SLA+R has been received, ACK has been returned
        case TW_ST_ARB_LOST_SLA_ACK:        // 0xB0:     GCA+R has been received, ACK has been returned
            // ST->SLA_ACK
            // We are being addressed as slave for reading (data must be transmitted back to master)
            // Fall-through to transmit first data byte
        case TW_ST_DATA_ACK:                // 0xB8: data byte has been transmitted, ACK has been received
            // ST->DATA_ACK
            // Transmit data byte
            TWDR = mcState->status;
            // End of data to write reached, expect NACK to data byte
            TWCR |= (1<<TWIE) | (1<<TWINT) | (0<<TWEA) | (1<<TWEN);
        case TW_ST_DATA_NACK:               // 0xC0: data byte has been transmitted, NACK has been received
        case TW_ST_LAST_DATA:               // 0xC8:
            // ST->DATA_NACK or LAST_DATA
            // All done
            // Switch to open slave
            TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;

            // Misc
        case TW_NO_INFO:                    // 0xF8: no relevant state information
            // Do nothing
            // NO_INFO
            break;
        case TW_BUS_ERROR:                  // 0x00: bus error due to illegal start or stop condition
            // BUS_ERROR
            // Reset internal hardware and release bus
            TWCR |= (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWSTO);
            break;
    }
}

