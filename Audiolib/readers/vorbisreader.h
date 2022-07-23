#pragma once

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
	VorbisReader(BinaryStream* stream);
	~VorbisReader() override;

	void read(float* buffer, int& bufferFrames) override;
	void reset() override;

	[[nodiscard]] bool atEnd() const override;
	[[nodiscard]] bool haveEnd() const override;
	[[nodiscard]] bool error() const override;

	[[nodiscard]] int numChannels() const override;
	[[nodiscard]] int sampleRate() const override;

	bool hasLooped() override;
};

#define ALIB_OGG_DETECT(out, stream, parameter) \
    do { \
        if(out) break; \
        char f[5] = {0}; \
        (stream)->read(f, 4); \
        (stream)->rewind(); \
        if(f[0]=='O'&&f[1]=='g'&&f[2]=='g'&&f[3]=='S') \
            (out) = new VorbisReader(parameter); \
    } while(0)
#else
#define ALIB_OGG_DETECT(out, stream, parameter)
#endif

}
