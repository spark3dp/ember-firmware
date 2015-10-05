//  File:   I2C_StreamBuffer.h
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

#ifndef I2C_STREAMBUFFER_H
#define	I2C_STREAMBUFFER_H

#include <streambuf>
#include <vector>

class I2C_StreamBuffer : public std::basic_streambuf<unsigned char>
{
public:
    I2C_StreamBuffer(unsigned char slaveAddress, int port);
    ~I2C_StreamBuffer();

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

