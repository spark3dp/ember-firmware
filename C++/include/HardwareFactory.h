#ifndef HARDWAREFACTORY_H
#define	HARDWAREFACTORY_H

#include <memory>

typedef std::unique_ptr<std::basic_streambuf<unsigned char> > StreamBufferPtr;

namespace HardwareFactory
{
StreamBufferPtr CreateMotorStreamBuffer();
StreamBufferPtr CreateFrontPanelStreamBuffer();
StreamBufferPtr CreateProjectorStreamBuffer();
};


#endif  // HARDWAREFACTORY_H
