#include "ringbuffer.h"

#include <string.h>
#include <stdlib.h>

namespace alib {

struct RingBuffer::Priv
{
    unsigned char *volatile writePtr, *volatile readPtr;
    unsigned char *buffer, *bufferEnd;
    int size;

    Priv(unsigned int bufferSize)
    {
        writePtr = readPtr = buffer = (unsigned char*)malloc(bufferSize);
        bufferEnd = buffer + bufferSize;
        size = bufferSize;
    }

    ~Priv()
    {
        free(buffer);
    }

    void writeContinous(const void *data, int &len)
    {
        unsigned char *r = readPtr, *w = writePtr;

        int maxContinousWrite = (r > w ? r - w : bufferEnd - w);

        if(len > maxContinousWrite)
            len = maxContinousWrite;

        if(r == buffer && w + len == bufferEnd || w + len == r)
            len -= 1;

        memcpy(writePtr, data, len);

        if(w + len == bufferEnd)
            writePtr = buffer;
        else
            writePtr += len;
    }

    void readContinous(void *outData, int &len)
    {
        unsigned char *r = readPtr, *w = writePtr;

        int maxContinousRead = (w >= r ? w - r : bufferEnd - r);

        if(len > maxContinousRead)
            len = maxContinousRead;

        memcpy(outData, readPtr, len);

        if(r + len == bufferEnd)
            readPtr = buffer;
        else
            readPtr += len;
    }

    void seek(int &len)
    {
        unsigned char *r = readPtr, *w = writePtr;
        int dataAvail = length(r, w);

        if(len > dataAvail)
            len = dataAvail;

        readPtr = buffer + ((len + abs(r - w)) % size);
    }

    int freeSpace(unsigned char *r, unsigned char *w)
    {
        if(r > w)
            return abs(r - w) - 1;
        else
            return size - abs(r - w) - 1;
    }

    int length(unsigned char *r, unsigned char *w)
    {
        if(r <= w)
            return abs(r - w);
        else
            return size - abs(r - w);
    }
};

RingBuffer::RingBuffer(unsigned int bufferSize)
    : p(new Priv(bufferSize))
{
}

RingBuffer::~RingBuffer()
{
    delete p;
}

int RingBuffer::write(const void *data, unsigned int len)
{
    int toWrite = len;
    int totalWritten = 0;
    int totalToWrite = len;

    for(int i = 0; i < 2; ++i)
    {
        p->writeContinous((char*)(data) + totalWritten, toWrite);

        totalWritten += toWrite;
        toWrite = totalToWrite - totalWritten;

        if(toWrite == 0)
            break;
    }

    return totalWritten;
}

int RingBuffer::read(void *outData, unsigned int len)
{
    int toRead = len;
    int totalRead = 0;
    int totalToRead = len;

    for(int i = 0; i < 2; ++i)
    {
        p->readContinous((char*)(outData) + totalRead, toRead);

        totalRead += toRead;
        toRead = totalToRead - totalRead;

        if(toRead == 0)
            break;
    }

    return totalRead;
}

int RingBuffer::seek(unsigned int len)
{
    int seekLen = len;
    p->seek(seekLen);
    return seekLen;
}

unsigned int RingBuffer::freeSpace()
{
    unsigned char *r = p->readPtr, *w = p->writePtr;

    return p->freeSpace(r, w);
}

unsigned int RingBuffer::length()
{
    unsigned char *r = p->readPtr, *w = p->writePtr;

    return p->length(r, w);
}

unsigned int RingBuffer::readPtr()
{
    return p->readPtr - p->buffer;
}

unsigned int RingBuffer::writePtr()
{
    return p->writePtr - p->buffer;
}


}
