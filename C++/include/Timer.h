/* 
 * File:   Timer.h
 * Author: Jason Lefley
 * 
 * Created on August 12, 2015, 12:00 PM
 */

#ifndef TIMER_H
#define	TIMER_H

#include "IResource.h"

class Timer : public IResource
{
public:
    Timer();
    ~Timer();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    void Start(double expirationTimeSeconds) const;
    double GetRemainingTimeSeconds() const;
    void Clear() const;
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    Timer(const Timer&);
    Timer& operator=(const Timer&);

private:
    int _fd;
    size_t _dataSize;
};

#endif	/* TIMER_H */

