#ifndef VORBISREADER_H
#define VORBISREADER_H

#include "audiolib.h"
#include "stream.h"
#include "buffer.h"
#include "samplereader.h"

namespace alib {

#ifdef ALIB_OGG_SUPPORT
class VorbisReader : public SampleReader, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    VorbisReader(BinaryStream *stream);
    ~VorbisReader();

    void read(float *buffer, int &bufferFrames);
    void reset();

    bool atEnd() const;
    bool haveEnd() const;
    bool error() const;

    int numChannels() const;
    int sampleRate() const;

    bool hasLooped();
};
#endif

#ifdef ALIB_OGG_SUPPORT
#define ALIB_OGG_DETECT(out, stream, parameter) \
    do { \
        if(out) break; \
        char f[5] = {0}; \
        stream->read(f, 4); \
        stream->rewind(); \
        if(f[0]=='O'&&f[1]=='g'&&f[2]=='g'&&f[3]=='S') \
            out = new VorbisReader(parameter); \
    } while(0)
#else
#define ALIB_OGG_DETECT(out, stream, parameter)
#endif

}

#endif // VORBISREADER_H
