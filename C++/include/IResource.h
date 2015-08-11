/* 
 * File:   IResource.h
 * Author: Jason Lefley
 * Interface to system resources for use with epoll
 *
 * Created on August 11, 2015, 10:33 AM
 */

#ifndef IRESOURCE_H
#define	IRESOURCE_H

#include <vector>
#include <string>

typedef std::string ResourceBuffer;
typedef std::vector<ResourceBuffer> ResourceBufferVec;

class IResource
{

public:

    virtual ~IResource() {}

    /*
     * Returns one "event's" worth of data from the resource
     * The return type is an array of buffers to accommodate the situation where the resource contains multiple
     * messages that a client needs to handle individually per event
     */
    virtual ResourceBufferVec Read() = 0;

    /*
     * Returns the epoll event types applicable to this resource
     */
    virtual uint32_t GetEventTypes() = 0;

    /*
     * Returns the file descriptor for this resource
     */ 
    virtual int GetFileDescriptor() = 0;

};


#endif	/* IRESOURCE_H */
