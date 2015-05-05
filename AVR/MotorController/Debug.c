/*
 * Debug.c
 * Author: Jason Lefley
 * Date  : 2015-05-01
 * Description: Debug output setup
 */

#include <avr/io.h> // uart.h needs RAMEND

#include "uart.h"
#include "Debug.h"

#define UART_BAUD_RATE 9600

static FILE uartStream;

void DebugInitialize()
{
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
    fdev_setup_stream(&uartStream, uart_putc, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartStream;
    stderr = &uartStream;
}

