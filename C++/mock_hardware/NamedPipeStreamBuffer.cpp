#include "mock_hardware/NamedPipeStreamBuffer.h"

#include <iostream>
#include <fcntl.h>
#include <stdexcept>

NamedPipeStreamBuffer::NamedPipeStreamBuffer(const std::string& readPipePath,
                                             const std::string& writePipePath) :
_lastRead(traits_type::eof())
{
    _readFd = open(readPipePath.c_str(), O_RDWR);
    std::cout << "pipe opened for reading" << std::endl;

    if (_readFd < 0)
    {
        throw std::runtime_error("unable to open pipe for reading");
    }
    
    _writeFd = open(writePipePath.c_str(), O_WRONLY | O_NONBLOCK);
    std::cout << "pipe opened for writing" << std::endl;

    if (_writeFd < 0)
    {
        close(_readFd);
        throw std::runtime_error("unable to open pipe for writing");
    }
}

NamedPipeStreamBuffer::~NamedPipeStreamBuffer()
{
    close(_writeFd);
    close(_readFd);
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::overflow(int_type ch)
{
    std::cout << "overflow" << std::endl;

    if (write(_writeFd, &ch, 1) == 1)
    {
        return ch;
    }

    return traits_type::eof();
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::underflow()
{
    std::cout << "underflow" << std::endl;
    return _lastRead;
}

NamedPipeStreamBuffer::int_type NamedPipeStreamBuffer::uflow()
{
    std::cout << "uflow" << std::endl;
    unsigned char buffer;
    if (read(_readFd, &buffer, 1) == 1)
    {
        _lastRead = traits_type::to_int_type(buffer);
    }
    else
    {
        _lastRead = traits_type::eof();
    }
    return _lastRead;
}
