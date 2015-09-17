//  File: Debug.cpp
//  Debug output setup
//
//  This file is part of the Ember Motor Controller firmware.
//
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

#ifdef DEBUG

#include <avr/io.h> // uart.h needs RAMEND

#include "Debug.h"
#include "uart.h"

#define UART_BAUD_RATE 9600

static FILE uartStream;

// stdio specific putc function to interface with uart
// Conversion of new line characters to appropriate serial newline and/or
// carriage return sequence can be done here if desired
// See example at http://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
int UARTPutChar(char c, FILE* stream)
{
    uart_putc(c);
    return 0;
}

// Initialize uart and direct standard out and standard error to uart
void Debug::Initialize()
{
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
    fdev_setup_stream(&uartStream, UARTPutChar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartStream;
    stderr = &uartStream;
}

#endif  // DEBUG
