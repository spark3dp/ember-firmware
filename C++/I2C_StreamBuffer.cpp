//  File:   I2C_StreamBuffer.cpp
//  Stream buffer implementation backed by an I2C device
//
//  References: http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
//              http://sourceforge.net/projects/scstream/
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include "I2C_StreamBuffer.h"

#include <cassert>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "ErrorMessage.h"

static const int BUFFER_SIZE = 32;

I2C_StreamBuffer::I2C_StreamBuffer(unsigned char slaveAddress, int port) :
_buffer(BUFFER_SIZE + 1, '0'),
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
    std::ptrdiff_t n = pptr() - pbase();
    bool success = (write(_fd, pbase(), n) == n);
    pbump(-n);
    return success;
}

int I2C_StreamBuffer::sync()
{
    return Flush() ? 0 : -1;
}

I2C_StreamBuffer::int_type I2C_StreamBuffer::overflow(int_type ch)
{
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

I2C_StreamBuffer::int_type I2C_StreamBuffer::underflow()
{
    return _lastRead;
}

I2C_StreamBuffer::int_type I2C_StreamBuffer::uflow()
{
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
