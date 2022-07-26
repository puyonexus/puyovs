#pragma once

#include "audiolib.h"

namespace alib {

class SampleReader {
public:
	virtual ~SampleReader();

	/**
	 * Reads at most bufferFrames frames into buffer. If less frames are returned,
	 * the exact number of frames returned will be written into bufferFrames. This
	 * does not necessarily indicate error or end-of-stream conditions.
	 */
	virtual void read(float* buffer, int& bufferFrames) = 0;

	/**
	 * Resets the reader.
	 */
	virtual void reset() = 0;

	/**
	 * Returns true if you are at the end of a stream. Note that end of stream
	 * conditions denote that there WILL NOT be more data, not just that there
	 * is currently no data to return.
	 */
	virtual bool atEnd() const = 0;

	/**
	 * Returns true if the reader can have an end-of-stream condition.
	 */
	virtual bool haveEnd() const = 0;

	/**
	 * Returns true if there is an error condition. This may be used to
	 * determine if construction or auxillary setup of the stream has failed.
	 */
	virtual bool error() const = 0;

	/**
	 * Returns the stream sample rate in Hz.
	 */
	virtual int sampleRate() const = 0;

	/**
	 * Returns the number of channels.
	 */
	virtual int numChannels() const = 0;

	virtual bool hasLooped() = 0;
};

}
