/* 
 * File:   I2C_Device.h
 * Author: Richard Greene
 * Created on March 13, 2014, 1:45 PM
 */

#ifndef I2C_DEVICE_H
#define	I2C_DEVICE_H

#include <Hardware.h>

#define BUF_SIZE            (32)
#define MAX_I2C_CMD_TRIES   (2)


/// Defines an I2C device at a specific slave address with which the BBB
/// can communicate
class I2C_Device
{
public:
    I2C_Device(unsigned char slaveAddress, int port = I2C2_PORT);
    ~I2C_Device();
    bool Write(unsigned char data);
    bool Write(unsigned char registerAddress, unsigned char data);
    bool Write(unsigned char registerAddress, const unsigned char* data, 
               int len);
    unsigned char Read(unsigned char registerAddress);
    
private:    
    // don't allow construction without specifying a slave address
    I2C_Device() {} 
    int _i2cFile;    // file descriptor for this device
    unsigned char _writeBuf[BUF_SIZE];	// contains data to be written
    unsigned char _readBuf[BUF_SIZE];	// contains data that was read
    bool _isNullDevice;          // if true, creates a dummy device
};


#endif	/* I2C_DEVICE_H */

