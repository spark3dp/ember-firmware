/* 
 * File:   HardwareTimeout.cpp
 * Author: Jason Lefley
 * 
 * Wrapper for timer resource that reads data from an I2C device
 * 
 * Created on August 12, 2015, 5:17 PM
 */

#include "HardwareTimeout.h"

#include "MotorController.h" // TODO remove after testing

HardwareTimeout::HardwareTimeout(Timer& timer) :
_timer(timer)
{
}

HardwareTimeout::~HardwareTimeout()
{
}

/*
 * Return the event types from the underlying timer resource
 */
uint32_t HardwareTimeout::GetEventTypes() const
{
    return _timer.GetEventTypes();
}

/*
 * Return the file descriptor from the underlying timer resource
 */
int HardwareTimeout::GetFileDescriptor() const
{
    return _timer.GetFileDescriptor();
}

/*
 * When a timeout occurs, read from both the timer and the hardware
 * Return the data from the hardware and discard the data from the timer
 */
ResourceBufferVec HardwareTimeout::Read()
{
    _timer.Read();

    // for testing
    ResourceBufferVec buffers;
    buffers.push_back(ResourceBuffer(MC_STATUS_SUCCESS, 1));
    return buffers;
}
