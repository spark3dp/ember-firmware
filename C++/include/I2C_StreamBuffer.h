/* 
 * File:   I2C_StreamBuffer.h
 * Author: Jason Lefley
 *
 * Created on October 1, 2015, 2:58 PM
 */

#ifndef I2C_STREAMBUFFER_H
#define	I2C_STREAMBUFFER_H

#include <streambuf>
#include <vector>

class I2C_StreamBuffer : public std::basic_streambuf<unsigned char>
{
public:
    I2C_StreamBuffer(unsigned char slaveAddress, int port);
    ~I2C_StreamBuffer();

    void Print();

private:
    I2C_StreamBuffer(const I2C_StreamBuffer&);
    I2C_StreamBuffer &operator=(const I2C_StreamBuffer&);
    bool Flush();
    int_type overflow(int_type ch);
    int sync();
    int_type underflow();
    int_type uflow();

    std::vector<unsigned char> _buffer;
    int _fd;
    int_type _lastRead;
};

#endif	/* I2C_STREAMBUFFER_H */

