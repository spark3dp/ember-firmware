/* 
 * File:   I_I2C_Device.h
 * Author: Jason Lefley
 *
 * Created on October 15, 2015, 2:00 PM
 */

#define MAX_I2C_CMD_TRIES    (2)

#ifndef I_I2C_DEVICE_H
#define	I_I2C_DEVICE_H

class I_I2C_Device
{
public:
    virtual ~I_I2C_Device() {}
    virtual bool Write(unsigned char data) const = 0;
    virtual bool Write(unsigned char registerAddress, unsigned char data) const = 0;
    virtual bool Write(unsigned char registerAddress, const unsigned char* data,
                       int length) const = 0;
    virtual unsigned char Read(unsigned char registerAddress) const = 0;
};

#endif  // I_I2C_DEVICE_H

