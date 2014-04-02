/*
 * File:   EventHandler.cpp
 * Author: Richard Greene
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <EventHandler.h>
#include <MessageStrings.h>
#include <PrintEngine.h>
#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <map>

/// Public constructor.
/// Initializes file descriptors for events (including hardware interrupt 
/// handlers and FIFOs for status and errors), and subscriber lists
EventHandler::EventHandler() 
{
    // initialize array of file descriptors to "empty" values
    for(int i = Undefined; i < MaxEventTypes; i++)
        _fileDescriptors[i] = -1;
        
    // initialize file descriptors for hardware interrupts, which are not
    // "owned" by any other component
    for(int i = ButtonInterrupt; i <= DoorInterrupt; i++)
    {
        _fileDescriptors[i] = GetInterruptDescriptor((EventType)i);
        if(_fileDescriptors[i] < 0)
            exit(-1);
    }
}

/// Closes file descriptors used for events (and deletes temporary ones))
EventHandler::~EventHandler()
{
    
}

// TODO: move this to a separate utility for reporting formatted error strings
char msg[100];
char* FormatError(const char * format, int value)
{
    sprintf(msg, format, value);
    return msg;
}

/// Allows a client to set the file descriptor used for an event
void EventHandler::SetFileDescriptor(EventType eventType, int fd)
{
    if(_fileDescriptors[eventType] >= 0)
    {
        perror(FormatError(FILE_DESCRIPTOR_IN_USE_ERROR, eventType));
        exit(-1);
    }
    _fileDescriptors[eventType] = fd;
}

/// Allows a client to subscribe to an event
void EventHandler::Subscribe(EventType eventType, CallbackInterface* pObject)
{
    Subscription subscription(eventType, pObject);
    _subscriptions[eventType].push_back(subscription);
}

/// Begin handling events, in an infinite loop.
void EventHandler::Begin()
{   
    int pollFd = epoll_create(MaxEventTypes);
    if (pollFd == -1 ) 
    {
        perror(EPOLL_CREATE_ERROR);
        exit(-1);
    }

    // epoll event structures for monitoring file descriptors
    struct epoll_event epollEvent[MaxEventTypes];   
    struct epoll_event events[MaxEventTypes];
    std::map<int, EventType> fdMap;
    
    // set up what epoll watches for and the file descriptors we care about
    for(int et = Undefined; et < MaxEventTypes; et++)
    {
        if(_fileDescriptors[et] < 0)
            continue;
        
        // set up the map from file descriptors to event types
        fdMap[_fileDescriptors[et]] = (EventType)et;
        
        epollEvent[et].events = EPOLLPRI | EPOLLERR | EPOLLET;
        epollEvent[et].data.fd = _fileDescriptors[et];

        if( epoll_ctl(pollFd, EPOLL_CTL_ADD, _fileDescriptors[et], &epollEvent[et]) != 0) 
        {
            perror(FormatError(EPOLL_SETUP_ERROR, et));
            exit(-1);
        }
    }
    
    // start handling epoll in loop with 10ms sleep, 
    // that calls all the subscribers for each event type
    for(;;)
    {
        int numFDs = epoll_wait( pollFd, events, MaxEventTypes, 0 );
        if(numFDs) // status is the number of file descriptors ready for the requested io
        {
            if(numFDs < 0)
            {
                // should this be a fatal error? perhaps only if it keeps repeating
                perror(FormatError(NEGATIVE_NUM_FDS_ERROR, numFDs));
            }
            for(int n = 0; n < numFDs; n++)
            {
                // we received events, so handle them
                int fd = events[n].data.fd;
                EventType et = fdMap[fd];
                
                // extra qualification for hardware interrupts
                if(et >= ButtonInterrupt && et <= DoorInterrupt)
                {
                    if(!(events[n].events & EPOLLPRI))
                        continue;
                    
                    char c;
                    lseek(fd,0,SEEK_SET);
                    read(fd, &c, 1);
                    if(c != '1')
                        continue;  // not a rising edge
                    
                    if(et == DoorInterrupt)
                    {
                        // TODO: may need to debounce this switch, 
                        // to avoid a whole series of interrupts
                    }
                }
                else  // qualification for timer and FIFO interrupts
                {
                    if(!(events[n].events & EPOLLIN))
                            continue;    
                } 
  
                void* data;  
                bool doCallbacks = true;
                switch(et)
                {
                    case ButtonInterrupt:
                    case MotorInterrupt:
                        // the recipient will need to read the board 
                        // to find the reason for the interrupt
                        break;
                        
                    case DoorInterrupt:
                        break;
                        
                    case PrintEngineDelayEnd:
                    case MotorTimeout:
                        break;
                        
                    case PrinterStatus:
                        // make sure we read the most current status
                        // and send that to all subscribers
                        int n;
                        struct PrinterStatus status;
                        struct PrinterStatus tempStatus;
                        lseek(fd,0,SEEK_SET);
                        do
                        {
                            n = read(fd, &tempStatus, sizeof(struct PrinterStatus)); 
                            if(n == sizeof(struct PrinterStatus))
                                status = tempStatus;
                        }
                        while(n == sizeof(struct PrinterStatus));
                            
                        data = &status;
                        //data = read from status FIFO
                        break;
                        
                    default:
                        // "impossible" case
                        perror(FormatError(UNEXPECTED_EVENT_ERROR, et));
                        doCallbacks = false;
                        break;
                }
                 
                // call back each of the subscribers to this event
                int numSubscribers = _subscriptions[et].size();
                if(doCallbacks)
                    for(int i = 0; i < numSubscribers; i++)
                       _subscriptions[et][i].Call(et, data);
            } 
        }
        
        // wait 10ms before checking epoll again
        usleep(10000); 
    }
}

/// Sets up a GPIO as a positive edge-triggered  interrupt
int EventHandler::GetInterruptDescriptor(EventType eventType)
{
    int inputPin = GetInputPinFor(eventType);
    
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
    if ((inputHandle = fopen("/sys/class/gpio/export", "ab")) == NULL)
    {
        perror(FormatError(GPIO_EXPORT_ERROR, inputPin));
        return -1;
    }
    strcpy(setValue, GPIOInputString);
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL)
    {
        perror(FormatError(GPIO_DIRECTION_ERROR, inputPin));
        return -1;
    }
    strcpy(setValue,"in");
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
    
    // set it to edge triggered
    if ((inputHandle = fopen(GPIOEdge, "rb+")) == NULL)
    {
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

/// Unexports GPIO pins used for hardware in
void EventHandler::UnexportPins()
{
    for(int et = ButtonInterrupt; et < MaxEventTypes; et++)
    {
        char GPIOInputString[4], setValue[4];
        FILE *inputHandle = NULL;
        
        int inputPin = GetInputPinFor((EventType)et);
        
        if ((inputHandle = fopen("/sys/class/gpio/unexport", "ab")) == NULL) 
        {
            perror(UNEXPORT_ERROR);
            exit(-1);
        }
        strcpy(setValue, GPIOInputString);
        fwrite(&setValue, sizeof(char), 2, inputHandle);
        fclose(inputHandle);
    }
}

/// Gets the GPIO pin used for hardware interrupts
int EventHandler::GetInputPinFor(EventType et)
{
    switch(et)
    {
        case ButtonInterrupt:
            return(MOTOR_INTERRUPT_PIN);  
            break;
            
        case MotorInterrupt:
            return(UI_INTERRUPT_PIN);  
            break;
            
        case DoorInterrupt:
            return(DOOR_INTERRUPT_PIN);  
            break;
            
        default:
            // "impossible" case
            perror(FormatError(INVALID_INTERRUPT_ERROR, et));
            exit(-1);
            break;
    }
}
