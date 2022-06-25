#ifndef PCMREADER_H
#define PCMREADER_H

#include "samplereader.h"
#include "buffer.h"

namespace alib
{

class PCMReader : public SampleReader, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    PCMReader(BinaryStream *dataStream, int channels, int freq);
    ~PCMReader();

    void read(float *buffer, int &bufferFrames);
    void reset();

    bool atEnd() const;
    bool haveEnd() const;
    bool error() const;

    int numChannels() const;
    int sampleRate() const;

    bool hasLooped();
};

}

#endif // PCMREADER_H
