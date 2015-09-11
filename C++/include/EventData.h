//  File:   EventData.h
//  Encapsulates event data using a discriminated type to store arbitrary data
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

#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <boost/any.hpp>

class EventData
{
public:
    EventData(const boost::any& data) : _data(data) {}
    ~EventData() {}
    
    template<typename T>
    T Get() const { return boost::any_cast<T>(_data); }

private:
    boost::any _data;
};

#endif    // EVENTDATA_H
