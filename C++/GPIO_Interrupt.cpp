/* 
 * File:   GPIO_Interrupt.cpp
 * Author: Jason Lefley
 *
 * Provides access to hardware interrupts through a GPIO pin
 *  
 * Created on August 13, 2015, 10:59 AM
 */

#include <fcntl.h>
#include <cerrno>
#include <stdexcept>
#include <sys/epoll.h>

#include "GPIO_Interrupt.h"
#include "Filenames.h"
#include "ErrorMessage.h"

// Disable the default optimization (-O2), which prevents opening GPIOs!
#pragma GCC optimize ("O0")

/*
 * Constructor, set up pin GPIO as interrupt, triggering on specified edge
 */
GPIO_Interrupt::GPIO_Interrupt(int pin, const std::string& edge) :
_pin(pin)
{
    // Setup GPIO as interrupt pin
    char GPIOInputString[4], GPIOInputValue[64], GPIODirection[64],
            GPIOEdge[64], setValue[10];
    FILE *inputHandle = NULL;
    
    // Setup input
    sprintf(GPIOInputString, "%d", pin);
    sprintf(GPIOInputValue, GPIO_VALUE, pin);
    sprintf(GPIODirection, GPIO_DIRECTION, pin);
    sprintf(GPIOEdge, GPIO_EDGE, pin);
    
    // Export & configure the pin
    if (!(inputHandle = fopen(GPIO_EXPORT, "ab")))
        throw std::runtime_error(ErrorMessage::Format(GpioExport, pin, errno));
    
    strcpy(setValue, GPIOInputString);
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
 
    // Set direction of the pin to an input
    if (!(inputHandle = fopen(GPIODirection, "rb+")))
        throw std::runtime_error(ErrorMessage::Format(GpioDirection, pin, errno));
    
    strcpy(setValue,"in");
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
    
    // Set it to edge triggered
    if (!(inputHandle = fopen(GPIOEdge, "rb+")))
        throw std::runtime_error(ErrorMessage::Format(GpioEdge, pin, errno));
    
    strcpy(setValue, edge.c_str());
    fwrite(&setValue, sizeof(char), edge.size(), inputHandle);
    fclose(inputHandle);

    // Open the file descriptor for the interrupt
    _fd = open(GPIOInputValue, O_RDONLY);

    if(_fd < 0)
        throw std::runtime_error(ErrorMessage::Format(GpioInterrupt, pin, errno));
    
    // Prevent initial spurious "interrupt"
    unsigned char data;
    read(_fd, &data, 1);
}

/*
 * Destructor
 * Attempt to un-export the pin, containing an exception if encountered
 */
GPIO_Interrupt::~GPIO_Interrupt()
{
    try
    {
        UnExport();
    }
    catch (const std::exception& e)
    {
        // Do not throw exceptions from destructor
        // All the destructor can do is print the message
        std::cerr << e.what() << std::endl;
    }

    close(_fd);
}

/*
 * Un-export the GPIO pin
 * Exists as a separate function so clients of this object can handle exception
 * if desired
 * The destructor calls this but catches the exception
 */
void GPIO_Interrupt::UnExport() const
{
    char GPIOInputString[4], setValue[4];
    FILE *inputHandle = NULL;

    sprintf(GPIOInputString, "%d", _pin);
    
    if (!(inputHandle = fopen(GPIO_UNEXPORT, "ab"))) 
        throw std::runtime_error(ErrorMessage::Format(GpioUnexport, errno));
    
    strcpy(setValue, GPIOInputString);
    fwrite(&setValue, sizeof(char), 2, inputHandle);
    fclose(inputHandle);
}

uint32_t GPIO_Interrupt::GetEventTypes() const
{
    return EPOLLPRI | EPOLLERR | EPOLLET;
}

int GPIO_Interrupt::GetFileDescriptor() const
{
    return _fd;
}

EventDataVec GPIO_Interrupt::Read()
{
    char buffer;
    EventDataVec eventData;

    lseek(_fd, 0, SEEK_SET);
    
    if (read(_fd, &buffer, 1) == 1)
        eventData.push_back(EventData(buffer));

    return eventData;
}

bool GPIO_Interrupt::QualifyEvents(uint32_t events) const
{
    return EPOLLPRI & events;
}

