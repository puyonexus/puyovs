#pragma once

#ifdef ALIB_VGMSTREAM_SUPPORT
#include "samplereader.h"

namespace alib {

class Buffer;
class BinaryStream;

class VgmReader : public SampleReader, NonCopyable
{
	ALIB_DECLARE_PRIV;

public:
	VgmReader(Buffer& buffer);
	VgmReader(BinaryStream* stream);
	~VgmReader() override;

	void read(float* buffer, int& bufferFrames) override;
	void reset() override;

	[[nodiscard]] bool atEnd() const override;
	[[nodiscard]] bool haveEnd() const override;
	[[nodiscard]] bool error() const override;

	[[nodiscard]] int numChannels() const override;
	[[nodiscard]] int sampleRate() const override;

	bool hasLooped() override;
};

}

#define ALIB_VGM_DETECT(out, stream, parameter) \
    do { \
        if(out) break; \
        (out) = new VgmReader(parameter); \
        if((out) && (out)->error()) { \
            delete (out); \
            (out) = 0; \
            (stream)->rewind(); \
        } \
    } while(0)
#else
#define ALIB_VGM_DETECT(out, stream, parameter)
#endif
