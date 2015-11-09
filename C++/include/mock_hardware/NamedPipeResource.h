//  File:   NamedPipeResource.h
//  Resource that reads data from a named pipe (for testing purposes)
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

#ifndef MOCKHARDWARE_NAMEDPIPERESOURCE_H
#define MOCKHARDWARE_NAMEDPIPERESOURCE_H

#include "IResource.h"

class NamedPipeResource : public IResource
{
public:
    NamedPipeResource(const std::string& filePath, size_t dataSize);
    ~NamedPipeResource();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    bool QualifyEvents(uint32_t events) const;

private:
    NamedPipeResource(const NamedPipeResource&);
    NamedPipeResource& operator=(const NamedPipeResource&);

    size_t _dataSize;
    int _fd;
};


#endif  // MOCKHARDWARE_NAMEDPIPERESOURCE_H

