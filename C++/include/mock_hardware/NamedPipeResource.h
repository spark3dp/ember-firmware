#ifndef MOCKHARDWARE_NAMEDPIPERESOURCE_H
#define MOCKHARDWARE_NAMEDPIPERESOURCE_H

#include "IResource.h"

class NamedPipeResource : public IResource
{
public:
    NamedPipeResource(const std::string& filePath, size_t dataSize);
    ~NamedPipeResource();
    uint32_t GetEventTypes() const;
    int GetFileDescriptor() const;
    EventDataVec Read();
    bool QualifyEvents(uint32_t events) const;

private:
    NamedPipeResource(const NamedPipeResource&);
    NamedPipeResource& operator=(const NamedPipeResource&);

    size_t _dataSize;
    int _fd;
};


#endif  // MOCKHARDWARE_NAMEDPIPERESOURCE_H

