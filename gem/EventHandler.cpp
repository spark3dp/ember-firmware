/*
 * File:   EventHandler.cpp
 * Author: Richard Greene
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <EventHandler.h>
#include <MessageStrings.h>
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
    for(int i = MotorInterrupt; i <= DoorInterrupt; i++)
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
void EventHandler::Subscribe(EventType eventType, EventCallback callback)
{
    _callbacks[eventType].push_back(callback);
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
    //that calls all the subscribers for each event type
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
                EventType et = fdMap[events[n].data.fd];
                
                if(et >= ButtonInterrupt && et <= DoorInterrupt)
                    if(!(fdMap[events[n].events & EPOLLPRI))
                        continue;
                
                //!!!!!!!!!!!!!!!!!!!!!!!!!
                // TODO: here we probably also need the other filtering common to hardware
                // interrupts (making sure its a '1' and anding with EPOLLPRI)
                // perh handle those in separate group before grand switch?
                ////////////////////////////////
                
  
                void* data = NULL;      
                switch(et)
                {
                    case ButtonInterrupt:
                        // read the board to find the reason for the interrupt
                        char reason;
                        // reason = uiBoard.Read(BUTTON_STATUS);
                        data = &reason;
                        break;
    
                
                    case MotorInterrupt:
                        // read the board to find the reason for the interrupt
                        // char reason;
                        // reason = motor.Read(MOTOR_STATUS);
                        data = &reason;
                        break;
                        
                    case DoorInterrupt:
                        // read the GPIO to find out if the door was opened or closed
                        // char reason;
                        // reason = readGPIO(DOOR_INTERRUPT_PIN);
                        data = &reason;
                        break;

                    default:
                        // "impossible" case
                        perror(FormatError(UNEXPECTED_EVENT_ERROR, et));
                        break;
                }
                 
                // call each of the subscribers to this event
                int numCallbacks = _callbacks[et].size();
                if(data != NULL)
                    for(int i = 0; i < numCallbacks; i++)
                       _callbacks[et][i](data);
                        
                // wait 10ms before checking epoll again
                usleep(10000); 
            }
        }
    }
// TODO: remove this old code                
//                if ( events[n].data.fd == _interruptFD && (events[0].events & EPOLLPRI) )
//                {
//                    // Get the file descriptor of the data that is ready, seek to the beginning
//                    // and read the data
//                    char c;
//                    printf("on interrupt pin\n");
//                    lseek(_interruptFD,0,SEEK_SET);
//                    read(_interruptFD, &c, 1);
//                    if(c == '1')
//                    {
//                        // write something to the named pipe
//                        char buf[100];
//                        sprintf(buf, "status = %d", numInterrupts);
//                        lseek(statusWriteFd,0,SEEK_SET);
//                        write(statusWriteFd, buf, strlen(buf));
//                    
//                        printf("status interrupt: %s\n", buf);
//                        
//                        
//                        
//                        printf("received button interrupt '%c' (%d)\n", c, c);
//                        usleep(100000); // pause for debounce
//                        printf("resuming after button press\n");
//
//                        // exit after 5 interrupts
//                        if(++numInterrupts > 4)
//                        {
//                            leave = true;
//                            break;
//                        }
//
//                        // if timer fired start the timer again
//                        if(timer1Fired)
//                        {
//                            if (timerfd_settime(_timer1FD, 0, &timer1Value, NULL) == -1)
//                            {
//                                printf("couldn't set time again\n");
//                                return(1);
//                            } 
//                            timer1Fired = false;
//                        }
//                    }
//                }
//                else if (events[n].data.fd == _timer1FD && (events[0].events & EPOLLIN))
//                {
//                    printf("timer 1 expired\n");
//                    timer1Fired = true;
//                }
//                else if (events[n].data.fd == _timer2FD && (events[0].events & EPOLLIN))
//                {
//                    printf("timer 2 expired\n");
//                }
//                else if(events[n].data.fd == statusReadFd)  // may need further qualification here
//                {
//                    char buf[100];
//                    lseek(statusReadFd,0,SEEK_SET);
//                    read(statusReadFd, buf, 100);
//                    printf("status interrupt: %s\n", buf);
//                }
//            }
//        }
// end of old code to be removed above
/////////////////////////////////////////////////////////  
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
