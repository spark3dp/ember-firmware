/* 
 * File:   I2C_Bus.cpp
 * Author: Richard Greene
 *
 * Implements a class that provides file descriptors for I2C buses.
 *  
 * Created on April 16, 2015, 4:04 PM
 */

#include <fcntl.h>
#include <stdio.h>
#include <cstdlib>

#include "I2C_Bus.h"
#include <Logger.h>
#include <ErrorMessage.h>

/// Gets the I2C_Bus singleton
I2C_Bus& I2C_Bus::Instance()
{
    static I2C_Bus i2cbus;

    return i2cbus;
}

/// Gets the file descriptor for the given I2C port, 
/// opening it if it isn't already open.
int I2C_Bus::GetFileForPort(int port)
{
    // see if this port has already been opened
    if(_ports.count(port) > 0)
        return _ports[port];
    
    // open the I2C port
    char s[20];
    sprintf(s, "//dev//i2c-%d", port);
    
    int fd = open(s, O_RDWR);
    if (fd < 0)
    {
        LOGGER.LogError(LOG_ERR, errno, ERR_MSG(I2cFileOpen));
        exit(1);
    }

    // record for reuse
    _ports[port] = fd;
    return fd;
}

/// Closes all open files.
I2C_Bus::~I2C_Bus() 
{
    for (std::map<int, int>::iterator it = _ports.begin(); it != _ports.end(); 
                                                                           ++it)
    {
        close(it->second);
    }
}

