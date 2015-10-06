//  File:   NamedPipeStreamBuffer.h
//  Stream buffer implementation backed by a pair of named pipes
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

#ifndef MOCKHARDWARE_NAMEDPIPESTREAMBUFFER_H
#define MOCKHARDWARE_NAMEDPIPESTREAMBUFFER_H

#include <streambuf>
#include <vector>

class NamedPipeStreamBuffer : public std::basic_streambuf<unsigned char>
{
public:
    NamedPipeStreamBuffer(const std::string& readPipePath,
                          const std::string& writePipePath);
    ~NamedPipeStreamBuffer();

private:
    NamedPipeStreamBuffer(const NamedPipeStreamBuffer&);
    NamedPipeStreamBuffer &operator=(const NamedPipeStreamBuffer&);
    int_type overflow(int_type ch);
    int_type underflow();
    int_type uflow();
    
    int _readFd;
    int _writeFd;
    int_type _lastRead;
};

#endif  // MOCKHARDWARE_NAMEDPIPESTREAMBUFFER_H

