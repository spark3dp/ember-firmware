/* 
 * File:   IResource.h
 * Author: Jason Lefley
 * Interface to file based system resources for use with epoll
 *
 * Created on August 11, 2015, 10:33 AM
 */

#ifndef IRESOURCE_H
#define	IRESOURCE_H

#include <vector>
#include <string>
#include <stdint.h>

typedef std::string ResourceBuffer;
typedef std::vector<ResourceBuffer> ResourceBufferVec;

class IResource
{

public:

    virtual ~IResource() {}

    /*
     * Returns one "event's" worth of data from the resource
     * The return type is an array of buffers to accommodate the situation where
     * the resource contains multiple messages that a client needs to handle
     * individually per event
     * In general, each data buffer does not contain a new-line or other
     * termination character
     */
    virtual ResourceBufferVec Read() = 0;

    /*
     * Returns the epoll event types applicable to this resource
     */
    virtual uint32_t GetEventTypes() const = 0;

    /*
     * Returns the file descriptor used to read from this resource
     */ 
    virtual int GetFileDescriptor() const = 0;

    /*
     * Compares the specified events with the events of concern to the resource
     * Returns true if the resource is concerned with the specified events and 
     * false otherwise
     */
    virtual bool QualifyEvents(uint32_t events) const = 0;
    
};


#endif	/* IRESOURCE_H */
