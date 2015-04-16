/* 
 * File:   I2C_Bus.h
 * Author: Richard Greene
 *
 * Defines a class that provides file descriptors for I2C buses.
 *  
 * Created on April 16, 2015, 4:04 PM
 */

#ifndef I2C_BUS_H
#define	I2C_BUS_H

#include <map>

#define I2C_BUS (I2C_Bus::Instance())

class I2C_Bus {
public:
    static I2C_Bus& Instance();
    int GetFileForPort(int port);

private:
    std::map<int, int> _ports;
    I2C_Bus() {};
    I2C_Bus(I2C_Bus const&);
    I2C_Bus& operator=(I2C_Bus const&);
    ~I2C_Bus();
};

#endif	/* I2C_BUS_H */

