/*
 * File:   EventHandler.cpp
 * Author: Richard Greene
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <stdio.h>
#include <stdlib.h>  
#include <fcntl.h>
#include <algorithm>
#include <iostream>
#include <sys/stat.h>

#include <EventHandler.h>
#include <Logger.h>
#include <Filenames.h>
#include <Shared.h>
#include <ErrorMessage.h>

// temporary
#include <cstring>

/// Public constructor.
/// Initializes file descriptors for events (including hardware interrupt 
/// handlers and FIFOs for status and errors), and subscriber lists
EventHandler::EventHandler() :
_pollFd(-1),
_commandReadFd(-1),
_commandWriteFd(-1)
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
        
//        if(et == Keyboard)
//        {
//            // initialize file descriptor for keyboard input, also not
//            // "owned" by any other component
//            _pEvents[et]->_fileDescriptor = STDIN_FILENO;
//        }
//        
//        if(et == UICommand)
//        {
//            // initialize file descriptor for UI command input, also not
//            // "owned" by any other component
//            // don't recreate the FIFO if it exists already
//            if (access(COMMAND_PIPE, F_OK) == -1) {
//                if (mkfifo(COMMAND_PIPE, 0666) < 0) {
//                  LOGGER.LogError(LOG_ERR, errno, ERR_MSG(CommandPipeCreation));
//                  exit(-1);  // we can't really run if we can't accept commands
//                }
//            }
//            // Open both ends within this process in non-blocking mode,
//            // otherwise open call would wait till other end of pipe
//            // is opened by another process
//            _commandReadFd = open(COMMAND_PIPE, O_RDONLY|O_NONBLOCK);
//            _commandWriteFd = open(COMMAND_PIPE, O_WRONLY|O_NONBLOCK);
//            
//            _pEvents[et]->_fileDescriptor = _commandReadFd;
//        }   
    } 
    
    _pollFd = epoll_create(MaxEventTypes);
    if (_pollFd == -1 ) 
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(EpollCreate));
        exit(-1);
    }
}

/// Deletes Events, unexports pins, and cleans up command pipe
EventHandler::~EventHandler()
{
    UnexportPins();

    for(int et = Undefined + 1; et < MaxEventTypes; et++)
        delete _pEvents[et];  
//    
//    if (access(COMMAND_PIPE, F_OK) != -1)
//        remove(COMMAND_PIPE);
    
    if (_pollFd != -1 ) 
        close(_pollFd);
//    if (_commandReadFd != -1 ) 
//        close(_commandReadFd);
//    if (_commandWriteFd != -1 ) 
//        close(_commandWriteFd);
}

/*
 * Add an event with an associated resource
 * The specified event type will arise when activity is detected on the specified resource
 */
void EventHandler::AddEvent(EventType eventType, IResource* pResource)
{
    epoll_event event;
    event.events = pResource->GetEventTypes();
    event.data.fd = pResource->GetFileDescriptor();
    event.data.u64 = eventType;
    _resources[eventType] = pResource;
    _fdMap[event.data.fd] = eventType;
    epoll_ctl(_pollFd, EPOLL_CTL_ADD, pResource->GetFileDescriptor(), &event);
}

/// Allows a client to set the file descriptor used for an event
void EventHandler::SetFileDescriptor(EventType eventType, int fd)
{
    if(_pEvents[eventType]->_fileDescriptor >= 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(FileDescriptorInUse), eventType);
        exit(-1);
    }
    _pEvents[eventType]->_fileDescriptor = fd;
}

/// Allows a client to set the I2C device and register used for some hardware
/// interrupts
void EventHandler::SetI2CDevice(EventType eventType, I2C_Device* pDevice,
                                unsigned char statusReg)
{
    _pEvents[eventType]->_pI2CDevice = pDevice;
    _pEvents[eventType]->_statusRegister = statusReg;
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

    // epoll event structures for monitoring file descriptors
    struct epoll_event epollEvent[MaxEventTypes];   
    struct epoll_event events[MaxEventTypes];
    //std::map<int, EventType> fdMap;

    // set up what epoll watches for and the file descriptors we care about
    for(int et = Undefined + 1; et < MaxEventTypes; et++)
    {
        if (et == UICommand || et == Keyboard || et == PrinterStatusUpdate)
            continue;

        if(_pEvents[et]->_fileDescriptor < 0)
        {
            // make sure there are no subscriptions for events not yet 
            // associated with a file descriptor
            if(_pEvents[et]->_subscriptions.size() > 0)
            {
                LOGGER.LogError(LOG_ERR, errno, ERR_MSG(NoFileDescriptor), et);
                exit(-1);
            }
            else
                continue;
        }
        
        // set up the map from file descriptors to event types
        _fdMap[_pEvents[et]->_fileDescriptor] = (EventType)et;

        epollEvent[et].events = _pEvents[et]->_inFlags;
        epollEvent[et].data.fd = _pEvents[et]->_fileDescriptor;

        if( epoll_ctl(_pollFd, EPOLL_CTL_ADD, _pEvents[et]->_fileDescriptor, 
                                             &epollEvent[et]) != 0) 
        {
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(EpollSetup), et);
            if(et != Keyboard)
                exit(-1);
        }
    }
    // start calling epoll in loop that calls all subscribers to each event type
    bool keepGoing = true;
    int numFDs = 0;
    while(keepGoing)
    {
        int timeout = -1;
#ifdef DEBUG
        // use 10 ms timeout for unit testing 
        if(!doForever)
            timeout = 10;
#endif              
        // Do a blocking epoll_wait, there's nothing to do until it returns
        numFDs = epoll_wait(_pollFd, events, MaxEventTypes, timeout);
        
        if(numFDs) // numFDs file descriptors are ready for the requested IO
        {
            if(numFDs < 0 && errno != EINTR)
            {
                // if this keeps repeating, it should probably be a fatal error
                LOGGER.LogError(LOG_WARNING, errno, ERR_MSG(NegativeNumFiles), 
                                 numFDs);
            }
            
            for(int n = 0; n < numFDs; n++)
            {
                // we received events, so handle them
                int fd = events[n].data.fd;
                EventType et = _fdMap[fd];
                
                // qualify the event
                // the event loop only cares about specific types of events, it might want to ignore some types of
                // events picked up by epoll (EPOLLERR, etc.)
                if(!(events[n].events & _pEvents[et]->_outFlags))
                    continue;
                
                // read the data associated with the event

                // UICommand events are set up using level triggering
                // If epoll indicates that the resource associated with UICommands is ready for reading, we can
                // read less than all the data contained and epoll_wait will still indicate that this resource is
                // ready for reading on the next iteration
                if(et == UICommand || et == Keyboard || et == PrinterStatusUpdate)
                {
//                    lseek(fd, 0, SEEK_SET);
//                    char buf;
//                    int i = 0;
//                    while(read(fd, &buf, 1) == 1)
//                    {
//                        _pEvents[et]->_data[i] = buf;
//                        if(buf == '\n' || buf == '\0')
//                        {
//                            // null terminate char array
//                            _pEvents[et]->_data[i] = '\0';
//                            break;
//                        }
//                        i++;
//                    }
                    IResource* resource = _resources[et];
                    ResourceBufferVec buffers = resource->Read();
                    for (ResourceBufferVec::iterator it = buffers.begin(); it != buffers.end(); it++)
                    {
                        ResourceBuffer buffer = *it;
                        char data[buffer.size() + 1]; // add one for null terminator
                        std::copy(buffer.begin(), buffer.end(), data);
                        data[buffer.size()] = '\0';
                        _pEvents[et]->CallSubscribers(et, data);
                    }
                }
                else
                {
                    lseek(fd, 0, SEEK_SET);
                    read(fd, _pEvents[et]->_data, _pEvents[et]->_numBytes);

                     // extra qualification for interrupts from motor & UI boards
                     // or motor board timeout
                    if((_pEvents[et]->_isHardwareInterrupt || et == MotorTimeout) && 
                       _pEvents[et]->_pI2CDevice != NULL)
                    {
                        // read the controller's status register & return that data 
                        // in the callback
                        _pEvents[et]->_data[0] = _pEvents[et]->_pI2CDevice->Read(
                                                    _pEvents[et]->_statusRegister);  
                    }
                    // call back each of the subscribers to this event
                    _pEvents[et]->CallSubscribers(et, _pEvents[et]->_data);
                }
//                else
//                {
//                    // read a line from stdin for keyboard commands
////                    char* line = (char*)(_pEvents[et]->_data);
////                    size_t linelen = 256;
////                    getline(&line, &linelen, stdin);
//                    ResourceBuffer data = stdIn.Read().front();
//                    std::cout << "got keyboard input: " << data << std::endl;
//                    std::copy(data.begin(), data.end(), _pEvents[et]->_data);
//                    _pEvents[et]->_data[data.size()] = '\n';
//                }
                                
                // extra qualification for interrupts from motor & UI boards
                // or motor board timeout
//                if((_pEvents[et]->_isHardwareInterrupt || et == MotorTimeout) && 
//                   _pEvents[et]->_pI2CDevice != NULL)
//                {
//                    // read the controller's status register & return that data 
//                    // in the callback
//                    _pEvents[et]->_data[0] = _pEvents[et]->_pI2CDevice->Read(
//                                                _pEvents[et]->_statusRegister);  
//                }
//                // call back each of the subscribers to this event
//                _pEvents[et]->CallSubscribers(et, _pEvents[et]->_data);
               
                // handleAllAvailableInput true for PrinterStatusUpdate
                // Send out all status updates before next event loop tick to ensure that current state is propagated
                // before handling new events
//                if(_pEvents[et]->_handleAllAvailableInput) 
//                {
//                    // handle all available input
//                    while(read(fd, _pEvents[et]->_data, _pEvents[et]->_numBytes) 
//                               == _pEvents[et]->_numBytes) 
//                    {
//                        // call back each of the subscribers to this event
//                        _pEvents[et]->CallSubscribers(et, _pEvents[et]->_data);
//                    }
//                }  
            } 
        }      
#ifdef DEBUG
        if(!doForever && --_numIterations < 0)
            keepGoing = false;
#endif        
    }
}

// disable the default optimization (-O2), which prevents opening GPIOs!
#pragma GCC optimize ("O0")

/// Sets up a GPIO as a positive edge-triggered  interrupt
int EventHandler::GetInterruptDescriptor(EventType eventType)
{
    int inputPin = GetInputPinFor(eventType);
    
    // setup GPIO as interrupt pin
    char GPIOInputString[4], GPIOInputValue[64], GPIODirection[64], 
         GPIOEdge[64], setValue[10];
    FILE *inputHandle = NULL;
    
    // setup input
    sprintf(GPIOInputString, "%d", inputPin);
    sprintf(GPIOInputValue, GPIO_VALUE, inputPin);
    sprintf(GPIODirection, GPIO_DIRECTION, inputPin);
    sprintf(GPIOEdge, GPIO_EDGE, inputPin);
    
    // export & configure the pin
    if ((inputHandle = fopen(GPIO_EXPORT, "ab")) == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioExport), inputPin);
        return -1;
    }
    strcpy(setValue, GPIOInputString);
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if ((inputHandle = fopen(GPIODirection, "rb+")) == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioDirection), inputPin);
        return -1;
    }
    strcpy(setValue,"in");
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
    
    // set it to edge triggered
    if ((inputHandle = fopen(GPIOEdge, "rb+")) == NULL)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioEdge), inputPin);
        return -1;
    }
    const char* edge = "rising";
    if(eventType == DoorInterrupt)
        edge = "both";  // we want events when door opens and closes
    else if(eventType == RotationInterrupt)
         edge = "falling";  // we only care when rotation to sensor is detected
    strcpy(setValue, edge);
    fwrite(&setValue, sizeof(char), strlen(edge), inputHandle);
    fclose(inputHandle);

    // Open the file descriptor for the interrupt
    int interruptFD = open(GPIOInputValue, O_RDONLY);
    if(interruptFD < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioInterrupt), inputPin);
        return -1;
    }  
    
    // prevent initial spurious "interrupt"
    unsigned char data;
    read(interruptFD, &data, 1);
    
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
            sprintf(GPIOInputString, "%d", inputPin);
            
            if ((inputHandle = fopen(GPIO_UNEXPORT, "ab")) == NULL) 
            {
                LOGGER.LogError(LOG_ERR, errno, ERR_MSG(GpioUnexport));
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
            return(DOOR_SENSOR_PIN);  
            break;
            
        case RotationInterrupt:
            return(ROTATION_SENSOR_PIN);  
            break;
                        
        default:
            // "impossible" case
            LOGGER.LogError(LOG_ERR, errno, ERR_MSG(InvalidInterrupt), et);
            exit(-1);
            break;
    }
}

