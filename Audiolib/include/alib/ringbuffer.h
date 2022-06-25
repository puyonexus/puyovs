#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "audiolib.h"

namespace alib {

class RingBuffer
{
    ALIB_DECLARE_PRIV;

public:
    RingBuffer(unsigned int bufferSize);
    ~RingBuffer();

    int write(const void *data, unsigned int len);
    int read(void *outData, unsigned int len);
    int seek(unsigned int len);

    unsigned int freeSpace();
    unsigned int length();

    unsigned int readPtr();
    unsigned int writePtr();
};

}

#endif // RINGBUFFER_H
