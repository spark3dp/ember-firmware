#ifndef HARDWAREFACTORY_H
#define	HARDWAREFACTORY_H

#include <memory>

class IResource;

typedef std::unique_ptr<std::basic_streambuf<unsigned char> > StreamBufferPtr;
typedef std::unique_ptr<IResource> ResourcePtr;

namespace HardwareFactory
{
StreamBufferPtr CreateMotorStreamBuffer();
StreamBufferPtr CreateFrontPanelStreamBuffer();
StreamBufferPtr CreateProjectorStreamBuffer();
ResourcePtr     CreateMotorControllerInterruptResource();
ResourcePtr     CreateFrontPanelInterruptResource();
};


#endif  // HARDWAREFACTORY_H
