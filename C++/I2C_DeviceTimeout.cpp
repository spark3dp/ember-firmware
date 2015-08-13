/* 
 * File:   I2C_DeviceTimeout.cpp
 * Author: Jason Lefley
 * 
 * Wrapper for timer resource that reads data from an I2C device
 * 
 * Created on August 12, 2015, 5:17 PM
 */

#include "I2C_DeviceTimeout.h"
#include "Timer.h"
#include "I2C_Device.h"

I2C_DeviceTimeout::I2C_DeviceTimeout(Timer& timer, I2C_Device& i2cDevice,
        unsigned char statusRegister) :
_timer(timer),
_i2cDevice(i2cDevice),
_statusRegister(statusRegister)
{
}

I2C_DeviceTimeout::~I2C_DeviceTimeout()
{
}

/*
 * Return the event types from the underlying timer resource
 */
uint32_t I2C_DeviceTimeout::GetEventTypes() const
{
    return _timer.GetEventTypes();
}

/*
 * Return the file descriptor from the underlying timer resource
 */
int I2C_DeviceTimeout::GetFileDescriptor() const
{
    return _timer.GetFileDescriptor();
}

/*
 * When a timeout occurs, read from both the timer and the hardware
 * Return the data from the hardware and discard the data from the timer
 */
ResourceBufferVec I2C_DeviceTimeout::Read()
{
    _timer.Read();

    ResourceBufferVec buffers;
    buffers.push_back(ResourceBuffer(1, _i2cDevice.Read(_statusRegister)));
    return buffers;
}
