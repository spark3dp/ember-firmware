/*! \file i2c.c \brief I2C interface using AVR Two-Wire Interface (TWI) hardware. */
//*****************************************************************************
//
// File Name	: 'i2c.c'
// Title		: I2C interface using AVR Two-Wire Interface (TWI) hardware
// Author		: Pascal Stang - Copyright (C) 2002-2003
// Created		: 2002.06.25
// Revised		: 2003.03.02
// Version		: 0.9
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "CommandBuffer.h"

#define I2C_ADDRESS 0x10

/*
 * Store the location of the controller status byte
 */
static MotorController_t *mcState;

// functions
void i2cInit(MotorController_t *mc)
{
    mcState = mc;
	
	// enable TWI (two-wire interface)
	sbi(TWCR, TWEN);
	
    // enable TWI interrupt and slave address ACK
	sbi(TWCR, TWIE);
	sbi(TWCR, TWEA);
	
    // set local device address (used in slave mode only)
	outb(TWAR, (I2C_ADDRESS<<1) | 0);
}

static void i2cReceiveByte(bool ackFlag)
{
	// begin receive over i2c
	if( ackFlag )
	{
		// ackFlag = TRUE: ACK the recevied data
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
	}
	else
	{
		// ackFlag = FALSE: NACK the recevied data
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
	}
}

//! I2C (TWI) interrupt service routine
SIGNAL(TWI_vect)
{
	// read status bits
	uint8_t status = inb(TWSR) & TWSR_STATUS_MASK;

	switch(status)
	{
	// Slave Receiver status codes
	case TW_SR_SLA_ACK:					// 0x60: own SLA+W has been received, ACK has been returned
	case TW_SR_ARB_LOST_SLA_ACK:		// 0x68: own SLA+W has been received, ACK has been returned
	case TW_SR_GCALL_ACK:				// 0x70:     GCA+W has been received, ACK has been returned
	case TW_SR_ARB_LOST_GCALL_ACK:		// 0x78:     GCA+W has been received, ACK has been returned
		// SR->SLA_ACK
		// we are being addressed as slave for writing (data will be received from master)
		// receive data byte and return ACK
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		break;
	case TW_SR_DATA_ACK:				// 0x80: data byte has been received, ACK has been returned
	case TW_SR_GCALL_DATA_ACK:			// 0x90: data byte has been received, ACK has been returned
		// SR->DATA_ACK
		
        // store previously received data byte in buffer
        commandBuffer.AddByte(inb(TWDR));

		// check receive buffer status
        if(commandBuffer.IsFull())
			// receive data byte and return NACK
			i2cReceiveByte(false);
		else
			// receive data byte and return ACK
			i2cReceiveByte(true);

		break;
	case TW_SR_DATA_NACK:				// 0x88: data byte has been received, NACK has been returned
	case TW_SR_GCALL_DATA_NACK:			// 0x98: data byte has been received, NACK has been returned
		// SR->DATA_NACK
		// receive data byte and return NACK
		i2cReceiveByte(false);
		//outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
		break;
	case TW_SR_STOP:					// 0xA0: STOP or REPEATED START has been received while addressed as slave
		// SR->SR_STOP
		// switch to SR mode with SLA ACK
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		// i2c receive is complete
		break;

	// Slave Transmitter
	case TW_ST_SLA_ACK:					// 0xA8: own SLA+R has been received, ACK has been returned
	case TW_ST_ARB_LOST_SLA_ACK:		// 0xB0:     GCA+R has been received, ACK has been returned
		// ST->SLA_ACK
		// we are being addressed as slave for reading (data must be transmitted back to master)
		// fall-through to transmit first data byte
	case TW_ST_DATA_ACK:				// 0xB8: data byte has been transmitted, ACK has been received
		// ST->DATA_ACK
		// transmit data byte
		//outb(TWDR, I2cSendData[I2cSendDataIndex++]);
        commandBuffer.RemoveLastByte();
		outb(TWDR, mcState->status);
        // end of data to write reached, expect NACK to data byte
        outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
	case TW_ST_DATA_NACK:				// 0xC0: data byte has been transmitted, NACK has been received
	case TW_ST_LAST_DATA:				// 0xC8:
		// ST->DATA_NACK or LAST_DATA
		// all done
		// switch to open slave
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		break;

	// Misc
	case TW_NO_INFO:					// 0xF8: No relevant state information
		// do nothing
		// NO_INFO
		break;
	case TW_BUS_ERROR:					// 0x00: Bus error due to illegal start or stop condition
		// BUS_ERROR
		// reset internal hardware and release bus
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTO)|BV(TWEA));
		break;
	}
}

