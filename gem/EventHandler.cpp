/*
 * File:   EventHandler.cpp
 * Author: Richard Greene
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <EventHandler.h>
#include <MessageStrings.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/// Public constructor.
/// Initializes file descriptors for events (including hardware interrupt 
/// handlers and FIFOs for status and errors), and subscriber lists
EventHandler::EventHandler() 
{
    for(EventType t = MotorInterrupt; t <= DoorInterrupt; t++)
    {
        _fileDescriptors[t] = GetInterruptDescriptor(t);
        if(_fileDescriptors[t] < 0)
            exit();
    }
    
}

/// Closes file descriptors used for events (and deletes temporary ones))
EventHandler::~EventHandler()
{
    
}

/// Allows a client to subscribe to an event
void EventHandler::Subscribe(EventType eventType, EventCallback callback)
{
    _callbacks[eventType].push_back(callback);
}

/// Begin handling events, in an infinite loop.
void EventHandler::Begin()
{
    // method that starts handling, with epoll in loop & 10ms sleep, 
    //that calls all the subscribers for each event type
    
}

// TODO: move this to a separate utility for reporting formatted error strings
char msg[100];
char* FormatError(const char * format, int value)
{
    sprintf(msg, format, value);
    return msg;
}

/// set up a GPIO as a positive edge-triggered  interrupt
int EventHandler::GetInterruptDescriptor(EventType eventType)
{
    int inputPin;
    switch(eventType)
    {
        case ButtonInterrupt:
            inputPin = MOTOR_INTERRUPT_PIN;  
            break;
            
        case MotorInterrupt:
            inputPin = UI_INTERRUPT_PIN;  
            break;
            
        case DoorOpened:
            inputPin = DOOR_INTERRUPT_PIN;  
            break;
            
        default:
            // "impossible" case
            perror(FormatError(INVALID_INTERRUPT_ERROR, eventType));
            return -1;
            break;
    }

    // setup GPIO as interrupt pin
    char GPIOInputString[4], GPIOInputValue[64], GPIODirection[64], 
         GPIOEdge[64], setValue[4];
    FILE *inputHandle = NULL;
    
    // setup input
    sprintf(GPIOInputString, "%d", inputPin);
    sprintf(GPIOInputValue, "/sys/class/gpio/gpio%d/value", inputPin);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", inputPin);
    sprintf(GPIOEdge, "/sys/class/gpio/gpio%d/edge", inputPin);
    
    // export & configure the pin
    if ((inputHandle = fopen("/sys/class/gpio/export", "ab")) == NULL){
        perror(FormatError(GPIO_EXPORT_ERROR, inputPin));
        return -1;
    }
    strcpy(setValue, GPIOInputString);
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL){
        perror(FormatError(GPIO_DIRECTION_ERROR, inputPin));
        return -1;
    }
    strcpy(setValue,"in");
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
    
    // set it to edge triggered
    if ((inputHandle = fopen(GPIOEdge, "rb+")) == NULL){
        perror(FormatError(GPIO_EDGE_ERROR, inputPin));
        return -1;
    }
    strcpy(setValue,"rising");
    fwrite(&setValue, sizeof(char), 6, inputHandle);
    fclose(inputHandle);

    // Open the file descriptor for the interrupt
    int interruptFD = open(GPIOInputValue, O_RDONLY);
    if(interruptFD < 0)
    {
        perror(FormatError(GPIO_INTERRUPT_ERROR, inputPin));
        return -1;
    }    
    return interruptFD;
}

