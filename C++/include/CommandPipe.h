/* 
 * File:   CommandPipe.h
 * Author: Jason Lefley
 *
 * Created on August 11, 2015, 4:37 PM
 */

#ifndef COMMANDPIPE_H
#define	COMMANDPIPE_H

#include "IResource.h"

class CommandPipe : public IResource
{
public:
    CommandPipe();
    ~CommandPipe();
    uint32_t GetEventTypes();
    int GetFileDescriptor();
    ResourceBufferVec Read();

private:
    int _readFd;
    int _writeFd;
};

#endif	/* COMMANDPIPE_H */
