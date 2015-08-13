/* 
 * File:   I2C_Resource.h
 * Author: Jason Lefley
 *
 * Created on August 12, 2015, 5:17 PM
 */

#ifndef I2C_RESOURCE_H
#define	I2C_RESOURCE_H

#include "IResource.h"

class I2C_Device;

class I2C_Resource : public IResource
{
public:
    I2C_Resource(IResource& resource, I2C_Device& i2cDevice,
            unsigned char readRegister);
    ~I2C_Resource();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();

private:
    // This class has reference members
    // Disable copy construction and copy assignment
    I2C_Resource(const I2C_Resource&);
    I2C_Resource& operator=(const I2C_Resource&);

private:
    IResource& _resource;
    I2C_Device& _i2cDevice;
    unsigned char _readRegister;
};

#endif	/* I2C_RESOURCE_H */

