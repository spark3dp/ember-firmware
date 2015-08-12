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
    uint32_t GetEventTypes();
    int GetFileDescriptor();
    ResourceBufferVec Read();
};

#endif	/* STANDARDIN_H */
