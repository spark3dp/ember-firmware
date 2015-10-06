#include "mock_hardware/NamedPipeResource.h"

#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <stdexcept>

NamedPipeResource::NamedPipeResource(const std::string& path, size_t dataSize) :
_dataSize(dataSize)
{
    _fd = open(path.c_str(), O_RDWR);

    if (_fd < 0)
    {
        throw std::runtime_error(
                "unable to open pipe for reading in NamedPipeResource");
    }
}

NamedPipeResource::~NamedPipeResource()
{
    close(_fd);
}

uint32_t NamedPipeResource::GetEventTypes() const
{
    return EPOLLIN;
}

int NamedPipeResource::GetFileDescriptor() const
{
    return _fd;
}

bool NamedPipeResource::QualifyEvents(uint32_t events) const
{
    return EPOLLIN & events;
}

EventDataVec NamedPipeResource::Read()
{
    unsigned char buffer;
    EventDataVec eventData;

    if (read(_fd, &buffer, _dataSize) == _dataSize)
    {
        eventData.push_back(EventData(buffer));
    }

    return eventData;
}
