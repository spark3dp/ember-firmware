/* 
 * File:   I2C_DeviceTimeout.h
 * Author: Jason Lefley
 *
 * Created on August 12, 2015, 5:17 PM
 */

#ifndef I2C_DEVICETIMEOUT_H
#define	I2C_DEVICETIMEOUT_H

#include "IResource.h"

class I2C_Device;
class Timer;

class I2C_DeviceTimeout : public IResource
{
public:
    I2C_DeviceTimeout(Timer& timer, I2C_Device& i2cDevice,
            unsigned char statusRegister);
    ~I2C_DeviceTimeout();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();

private:
    // This class has reference members
    // Disable copy construction and copy assignment
    I2C_DeviceTimeout(const I2C_DeviceTimeout&);
    I2C_DeviceTimeout& operator=(const I2C_DeviceTimeout&);

private:
    Timer& _timer;
    I2C_Device& _i2cDevice;
    unsigned char _statusRegister;
};

#endif	/* I2C_DEVICETIMEOUT_H */

