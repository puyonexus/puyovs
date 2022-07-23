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
        writePtr = readPtr = buffer = static_cast<unsigned char*>(malloc(bufferSize));
        bufferEnd = buffer + bufferSize;
        size = bufferSize;
    }

    ~Priv()
    {
        free(buffer);
    }

    void writeContinuous(const void *data, int &len)
    {
	    const unsigned char *r = readPtr;
	    const unsigned char *w = writePtr;

        const int maxContinuousWrite = static_cast<int>(r > w ? r - w : bufferEnd - w);

        if(len > maxContinuousWrite)
        {
	        len = maxContinuousWrite;
        }

        if(r == buffer && w + len == bufferEnd || w + len == r)
        {
	        len -= 1;
        }

        memcpy(writePtr, data, len);

        if(w + len == bufferEnd)
        {
	        writePtr = buffer;
        }
        else
        {
	        writePtr += len;
        }
    }

    void readContinuous(void *outData, int &len)
    {
        unsigned char *r = readPtr, *w = writePtr;

        const int maxContinuousRead = static_cast<int>(w >= r ? w - r : bufferEnd - r);

        if(len > maxContinuousRead)
        {
	        len = maxContinuousRead;
        }

        memcpy(outData, readPtr, len);

        if(r + len == bufferEnd)
        {
	        readPtr = buffer;
        }
        else
        {
	        readPtr += len;
        }
    }

    void seek(int &len)
    {
        unsigned char *r = readPtr, *w = writePtr;
        const int dataAvail = length(r, w);

        if(len > dataAvail)
        {
	        len = dataAvail;
        }

        readPtr = buffer + ((len + abs(r - w)) % size);
    }

    int freeSpace(unsigned char *r, unsigned char *w) const
    {
        if(r > w)
        {
	        return static_cast<int>(abs(r - w) - 1);
        }

        return static_cast<int>(size - abs(r - w) - 1);
    }

    int length(unsigned char *r, unsigned char *w) const
    {
        if(r <= w)
        {
	        return static_cast<int>(abs(r - w));
        }

        return static_cast<int>(size - abs(r - w));
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
    const int totalToWrite = len;

    for(int i = 0; i < 2; ++i)
    {
        p->writeContinuous(static_cast<const char*>(data) + totalWritten, toWrite);

        totalWritten += toWrite;
        toWrite = totalToWrite - totalWritten;

        if(toWrite == 0)
        {
	        break;
        }
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
        p->readContinuous(static_cast<char*>(outData) + totalRead, toRead);

        totalRead += toRead;
        toRead = totalToRead - totalRead;

        if(toRead == 0)
        {
	        break;
        }
    }

    return totalRead;
}

int RingBuffer::seek(const unsigned int len)
{
    int seekLen = len;
    p->seek(seekLen);
    return seekLen;
}

unsigned int RingBuffer::freeSpace() const
{
    unsigned char *r = p->readPtr, *w = p->writePtr;

    return p->freeSpace(r, w);
}

unsigned int RingBuffer::length() const
{
    unsigned char *r = p->readPtr, *w = p->writePtr;

    return p->length(r, w);
}

unsigned int RingBuffer::readPtr() const
{
    return static_cast<unsigned int>(p->readPtr - p->buffer);
}

unsigned int RingBuffer::writePtr() const
{
    return static_cast<unsigned int>(p->writePtr - p->buffer);
}


}
