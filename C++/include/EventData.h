/* 
 * File:   EventData.h
 * Author: Jason Lefley
 * 
 * Encapsulates event data using a discriminated type to store arbitrary data.
 *
 * Created on August 20, 2015, 10:46 AM
 */

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

#endif /* EVENTDATA_H */
