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
#include <fcntl.h>
#include <map>
#include <algorithm>
#include <iostream>

// uncomment the following line to instrument the code, 
// setting pin P8-18 high when sleeping
//#define INSTRUMENT

#ifdef INSTRUMENT
void InitLED();
void LigthLED(bool on);
#endif

/// Public constructor.
/// Initializes file descriptors for events (including hardware interrupt 
/// handlers and FIFOs for status and errors), and subscriber lists
EventHandler::EventHandler() 
{
    // initialize array of Events with file descriptors set to "empty" values
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
    {
        _pEvents[et] = new Event((EventType)et);
        _pEvents[et]->_fileDescriptor = -1;
        
        // initialize file descriptors for hardware interrupts, which are not
        // "owned" by any other component
        if(_pEvents[et]->_isHardwareInterrupt)
        {
            _pEvents[et]->_fileDescriptor = GetInterruptDescriptor((EventType)et);
            if(_pEvents[et]->_fileDescriptor < 0)
                exit(-1);
        }
        
        if(et == Keyboard)
        {
            // initialize file descriptor for keyboard input, also not
            // "owned" by any other component
            _pEvents[et]->_fileDescriptor = STDIN_FILENO;
        }
    }
#ifdef INSTRUMENT
    InitLED(); 
#endif    
}

/// Deletes Events and unexports pins
EventHandler::~EventHandler()
{
    UnexportPins();

    for(int et = Undefined + 1; et < MaxEventTypes; et++)
        delete _pEvents[et];  
}

/// Allows a client to set the file descriptor used for an event
void EventHandler::SetFileDescriptor(EventType eventType, int fd)
{
    if(_pEvents[eventType]->_fileDescriptor >= 0)
    {
        perror(FormatError(FILE_DESCRIPTOR_IN_USE_ERROR, eventType));
        exit(-1);
    }
    _pEvents[eventType]->_fileDescriptor = fd;
}

/// Allows a client to subscribe to an event
void EventHandler::Subscribe(EventType eventType, ICallback* pObject)
{
    _pEvents[eventType]->_subscriptions.push_back(pObject);
}

#ifdef DEBUG
int _numIterations = 0;
// Debug only version of Begin allows unit testing with a finite number of iterations
void EventHandler::Begin(int numIterations)
{
    _numIterations = numIterations;  
    Begin();
}
#endif 
    
/// Begin handling events, in an infinite loop.
void EventHandler::Begin()
{   
#ifdef DEBUG
    // do repeatedly if _numIterations is zero 
    bool doForever = _numIterations == 0;
#endif    
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
    int maxSize = 1;
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
    {
        if(_pEvents[et]->_fileDescriptor < 0)
        {
            // make sure there are no subscriptions for events not yet 
            // associated with a file descriptor
            if(_pEvents[et]->_subscriptions.size() > 0)
            {
                perror(NO_FILE_DESCRIPTOR_ERROR);
                exit(-1);
            }
            else
                continue;
        }
        
        // set up the map from file descriptors to event types
        fdMap[_pEvents[et]->_fileDescriptor] = (EventType)et;
        
        epollEvent[et].events = _pEvents[et]->_inFlags;
        epollEvent[et].data.fd = _pEvents[et]->_fileDescriptor;

        if( epoll_ctl(pollFd, EPOLL_CTL_ADD, _pEvents[et]->_fileDescriptor, &epollEvent[et]) != 0) 
        {
            perror(FormatError(EPOLL_SETUP_ERROR, et));
            exit(-1);
        }
        maxSize = std::max(maxSize, _pEvents[et]->_numBytes);
    }
    
    // start handling epoll in loop with 10ms sleep, 
    // that calls all the subscribers for each event type
    unsigned char* tempBuf = new unsigned char[maxSize];
    bool keepGoing = true;
    while(keepGoing)
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
                
                // qualify the event
                if(!(events[n].events & _pEvents[et]->_outFlags))
                    continue;
                
                // read the data associated with the event
                if(et != Keyboard)
                {
                    lseek(fd, 0, SEEK_SET);
                    read(fd, _pEvents[et]->_data, _pEvents[et]->_numBytes);
                }
                else
                {
                    // read a line from stdin for keyboard commands
                    char* line = (char*)(_pEvents[et]->_data);
                    size_t linelen = 256;
                    getline(&line, &linelen, stdin);
                    std::cout << "in event handler line = " << line << " " <<  (char*)(_pEvents[et]->_data) << std::endl;
                }
                
                if(_pEvents[et]->_ignoreAllButLatest)
                {
                    // discard all but the most recent input
                    while(read(fd, tempBuf, _pEvents[et]->_numBytes) == _pEvents[et]->_numBytes)
                       memcpy(_pEvents[et]->_data, tempBuf, _pEvents[et]->_numBytes);
                }
                
                // extra qualification for hardware interrupts
                if(_pEvents[et]->_isHardwareInterrupt && et != DoorInterrupt &&
                   _pEvents[et]->_data[0] != '1')
                        continue;  // not a rising edge
     
                // call back each of the subscribers to this event
                _pEvents[et]->CallSubscribers(et, _pEvents[et]->_data);
            } 
        }
        else
        {
            
#ifdef INSTRUMENT
            LigthLED(false); 
#endif         
            // no events this time, so wait 10ms before checking epoll again
            usleep(10000); 
#ifdef INSTRUMENT
            LigthLED(true); 
#endif                            
        }
        
#ifdef DEBUG
        if(!doForever && --_numIterations < 0)
            keepGoing = false;
#endif        
    }
    delete [] tempBuf;
}

// disable the default optimation (-O2), which prevents opening GPIOs!
#pragma GCC optimize ("O0")

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
    const char* edge = "rising";
    if(eventType == DoorInterrupt)
        edge = "both";  // we want events when door opens and closes
    strcpy(setValue, edge);
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

/// Unexports GPIO pins used for hardware interrupts
void EventHandler::UnexportPins()
{
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
    {
        if(_pEvents[et]->_isHardwareInterrupt)
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
}

/// Gets the GPIO pin used for hardware interrupts
int EventHandler::GetInputPinFor(EventType et)
{
    switch(et)
    {
        case ButtonInterrupt:
            return(UI_INTERRUPT_PIN);  
            break;
            
        case MotorInterrupt:
            return(MOTOR_INTERRUPT_PIN);  
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

#ifdef INSTRUMENT
FILE *myOutputHandle = NULL;
char GPIOOutputString[4], GPIOOutputValue[64];

// set up LED output
void InitLED()
{
    ////////////////////////////////////////////////////
    // setup output for LED
    int OutputPin=65; // pin P8-18
    
    char GPIODirection[64], setValue[4];
    
    sprintf(GPIOOutputString, "%d", OutputPin);
    sprintf(GPIOOutputValue, "/sys/class/gpio/gpio%d/value", OutputPin);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", OutputPin);
 
    // Export the pin
    if ((myOutputHandle = fopen("/sys/class/gpio/export", "ab")) == NULL){
        printf("Unable to export GPIO pin\n");
        exit(1);
    }
    strcpy(setValue, GPIOOutputString);
    fwrite(&setValue, sizeof(char), 2, myOutputHandle);
    fclose(myOutputHandle);
 
    // Set direction of the pin to an output
    if ((myOutputHandle = fopen(GPIODirection, "rb+")) == NULL){
        printf("Unable to open direction handle\n");
        exit(1);
    }
    strcpy(setValue,"out");
    fwrite(&setValue, sizeof(char), 3, myOutputHandle);
    fclose(myOutputHandle);
}

void LigthLED(bool on)
{
    char setValue[4];
    if ((myOutputHandle = fopen(GPIOOutputValue, "rb+")) == NULL){
        printf("Unable to open value handle to turn LED on\n");
        return;
    }
    strcpy(setValue, on ? "1" : "0"); 
    fwrite(&setValue, sizeof(char), 1, myOutputHandle);
    fclose(myOutputHandle);
}

#endif
