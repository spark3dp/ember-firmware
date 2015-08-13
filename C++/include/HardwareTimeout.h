/* 
 * File:   HardwareTimeout.h
 * Author: Jason Lefley
 *
 * Created on August 12, 2015, 5:17 PM
 */

#ifndef HARDWARETIMEOUT_H
#define	HARDWARETIMEOUT_H

#include "IResource.h"
#include "Timer.h"

class HardwareTimeout : public IResource
{
public:
    HardwareTimeout(Timer& timer);
    ~HardwareTimeout();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();

private:
    // This class has reference members
    // Disable copy construction and copy assignment
    HardwareTimeout(const HardwareTimeout&);
    HardwareTimeout& operator=(const HardwareTimeout&);

private:
    Timer& _timer;
};

#endif	/* HARDWARETIMEOUT_H */

