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
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    ResourceBufferVec Read();
    bool QualifyEvents(uint32_t events) const;

private:
    // This class owns a file based resource
    // Disable copy construction and copy assignment
    CommandPipe(const CommandPipe&);
    CommandPipe& operator=(const CommandPipe&);

private:
    int _readFd;
    int _writeFd;
    uint32_t _events;
};

#endif	/* COMMANDPIPE_H */
