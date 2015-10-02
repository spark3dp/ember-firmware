// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
#include "I2C_StreamBuffer.h"

#include <iostream>

#include <cassert>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "ErrorMessage.h"

static const int BUFFER_SIZE = 10;

I2C_StreamBuffer::I2C_StreamBuffer(unsigned char slaveAddress, int port) :
_buffer(BUFFER_SIZE, '0'),
_lastRead(traits_type::eof())
{
    unsigned char* base = &_buffer.front();
    setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier

    std::ostringstream pathStream;
    pathStream << "/dev/i2c-" << port;
    std::string path = pathStream.str();
   
    // open the I2C port
    _fd = open(path.c_str(), O_RDWR);

    if (_fd < 0)
    {
        throw std::runtime_error(ErrorMessage::Format(I2cFileOpen, errno));
    }

    // set the slave address for this device
    if (ioctl(_fd, I2C_SLAVE, slaveAddress) < 0)
    {
        close(_fd);
        throw std::runtime_error(ErrorMessage::Format(I2cSlaveAddress, errno));
    }
}

I2C_StreamBuffer::~I2C_StreamBuffer()
{
    close(_fd);
}

bool I2C_StreamBuffer::Flush()
{
    std::cout << "Flush" << std::endl;
    std::ptrdiff_t n = pptr() - pbase();
    bool success = (write(_fd, pbase(), n) == n);
    pbump(-n);
    return success;
}

int I2C_StreamBuffer::sync()
{
    std::cout << "sync" << std::endl;
    return Flush() ? 0 : -1;
}

I2C_StreamBuffer::int_type I2C_StreamBuffer::overflow(int_type ch)
{
    std::cout << "overflow" << std::endl;
    if (ch != traits_type::eof())
    {
        assert(std::less_equal<unsigned char *>()(pptr(), epptr()));
        *pptr() = ch;
        pbump(1);
        if (Flush())
        {
            return ch;
        }
    }

    return traits_type::eof();
}

void I2C_StreamBuffer::Print()
{
    std::cout << "[";
    for (int i = 0; i < BUFFER_SIZE - 1; i++)
    {
        std::cout << _buffer[i] << ", ";
    }
    std::cout << _buffer[BUFFER_SIZE - 1] << "]" << std::endl;
}

I2C_StreamBuffer::int_type I2C_StreamBuffer::underflow()
{
    std::cout << "underflow" << std::endl;
    return _lastRead;
}

I2C_StreamBuffer::int_type I2C_StreamBuffer::uflow()
{
    std::cout << "uflow" << std::endl;
    unsigned char buffer;
    if (read(_fd, &buffer, 1) == 1)
    {
        _lastRead = traits_type::to_int_type(buffer);
    }
    else
    {
        _lastRead = traits_type::eof();
    }
    return _lastRead;
}
