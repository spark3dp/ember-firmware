/* 
 * File:   Signals.h
 * Author: Jason Lefley
 *
 * Created on August 17, 2015, 12:42 PM
 */

#ifndef SIGNALS_H
#define	SIGNALS_H

#include "IResource.h"

class Signals : public IResource
{
public:
    Signals();
    ~Signals();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    bool QualifyEvents(uint32_t events) const;
    ResourceBufferVec Read();
 
private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    Signals(const Signals&);
    Signals& operator=(const Signals&);

private:
    size_t _dataSize;
    int _fd;
};

#endif	/* SIGNALS_H */

