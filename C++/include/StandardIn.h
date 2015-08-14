/* 
 * File:   StandardIn.h
 * Author: Jason Lefley
 *
 * Created on August 11, 2015, 11:22 AM
 */

#ifndef STANDARDIN_H
#define	STANDARDIN_H

#include "IResource.h"

class StandardIn : public IResource
{
public:
    StandardIn();
    ~StandardIn();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();
    bool QualifyEvents(uint32_t events) const;

private:
    uint32_t _events;
};

#endif	/* STANDARDIN_H */
