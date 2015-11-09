#ifndef NULLI2C_DEVICE_HPP
#define	NULLI2C_DEVICE_HPP

#include "I_I2C_Device.h"

class NullI2C_Device : public I_I2C_Device
{
public:
    NullI2C_Device() {}
    ~NullI2C_Device() {}

    bool Write(unsigned char data) const { return true; }
    bool Write(unsigned char registerAddress, unsigned char data) const
        { return true; }
    bool Write(unsigned char registerAddress, const unsigned char* data, 
               int length) const { return true; }
    unsigned char Read(unsigned char registerAddress) const { return 0x00; }

private:
    NullI2C_Device(const NullI2C_Device&);
    NullI2C_Device &operator=(const NullI2C_Device&);
};

#endif	/* NULLI2C_DEVICE_HPP */

