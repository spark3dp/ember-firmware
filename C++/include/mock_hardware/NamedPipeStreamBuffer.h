#ifndef NAMEDPIPESTREAMBUFFER_H
#define NAMEDPIPESTREAMBUFFER_H

#include <streambuf>
#include <vector>

class NamedPipeStreamBuffer : public std::basic_streambuf<unsigned char>
{
public:
    NamedPipeStreamBuffer(const std::string& readPipePath,
                          const std::string& writePipePath);
    ~NamedPipeStreamBuffer();

private:
    NamedPipeStreamBuffer(const NamedPipeStreamBuffer&);
    NamedPipeStreamBuffer &operator=(const NamedPipeStreamBuffer&);
    int_type overflow(int_type ch);
    int_type underflow();
    int_type uflow();
    
    int _readFd;
    int _writeFd;
    int_type _lastRead;
};

#endif  // NAMEDPIPESTREAMBUFFER_H

