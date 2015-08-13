/* 
 * File:   I2C_Resource.cpp
 * Author: Jason Lefley
 * 
 * Wrapper for resource that reads data from an I2C device
 * 
 * Created on August 12, 2015, 5:17 PM
 */

#include "I2C_Resource.h"
#include "Timer.h"
#include "I2C_Device.h"

I2C_Resource::I2C_Resource(IResource& resource, I2C_Device& i2cDevice,
        unsigned char readRegister) :
_resource(resource),
_i2cDevice(i2cDevice),
_readRegister(readRegister)
{
}

I2C_Resource::~I2C_Resource()
{
}

/*
 * Return the event types from the underlying timer resource
 */
uint32_t I2C_Resource::GetEventTypes() const
{
    return _resource.GetEventTypes();
}

/*
 * Return the file descriptor from the underlying timer resource
 */
int I2C_Resource::GetFileDescriptor() const
{
    return _resource.GetFileDescriptor();
}

/*
 * When a resource event occurs, read from both the underlying resource and the
 * I2C device
 * Return the data from the I2C device and discard the data from the underlying
 * resource
 */
ResourceBufferVec I2C_Resource::Read()
{
    _resource.Read();

    ResourceBufferVec buffers;
    buffers.push_back(ResourceBuffer(1, _i2cDevice.Read(_readRegister)));
    return buffers;
}
