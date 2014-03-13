/* 
 * File:   I2C_Device.h
 * Author: Richard Greene
 * Created on March 13, 2014, 1:45 PM
 */

#ifndef I2C_DEVICE_H
#define	I2C_DEVICE_H

#define I2C_PORT (1) // corresponds to I2C2 on BBB, P9 pins 19 & 20

#define BUF_SIZE 128

/// Defines an I2C device at a specific slave address with which the BBB
/// can communicate
class I2C_Device
{
public:
    I2C_Device(unsigned char slaveAddress);
    ~I2C_Device();
    void Write(unsigned char registerAddress, unsigned char data);
    unsigned char Read(unsigned char registerAddress);
    
private:
    int _i2cFile;    // file descriptor for this device
    unsigned char _writeBuf[BUF_SIZE];	// contains data to be written
	unsigned char _readBuf[BUF_SIZE];	// contains data that was read
    
    // don't allow construction without specifying a slave address
    I2C_Device() {} 
};


#endif	/* I2C_DEVICE_H */

