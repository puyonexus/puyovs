#ifndef VGMREADER_H
#define VGMREADER_H

#ifdef ALIB_VGMSTREAM_SUPPORT
#include "samplereader.h"

namespace alib {

class Buffer;
class BinaryStream;

class VgmReader : public SampleReader, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    VgmReader(Buffer &buffer);
    VgmReader(BinaryStream *stream);
    ~VgmReader();

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
#define ALIB_VGM_DETECT(out, stream, parameter) \
    do { \
        if(out) break; \
        out = new VgmReader(parameter); \
        if(out && out->error()) { \
            delete out; \
            out = 0; \
            stream->rewind(); \
        } \
    } while(0)
#else
#define ALIB_VGM_DETECT(out, stream, parameter)
#endif



#endif // VGMREADER_H
