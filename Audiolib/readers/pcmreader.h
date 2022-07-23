#pragma once

#include "samplereader.h"
#include "buffer.h"

namespace alib
{

class PCMReader : public SampleReader, NonCopyable
{
	ALIB_DECLARE_PRIV;

public:
	PCMReader(BinaryStream* dataStream, int channels, int freq);
	~PCMReader() override;

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
