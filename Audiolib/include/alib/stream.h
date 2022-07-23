#pragma once

#include "audiolib.h"
#include "buffer.h"
#include <stdio.h>
#include <vector>

namespace alib {

class StreamObserver;

class Stream
{
	ALIB_DECLARE_SHARED;

	friend class StreamObserver;

public:
	/**
	 * Creates an empty, invalid stream.
	 *
	 * The stream will be born with an error condition.
	 */
	Stream();

	/**
	 * Creates a stream which reads data from a binary buffer.
	 * @param buffer Buffer to read data from.
	 */
	Stream(Buffer& buffer);

	/**
	 * Creates a stream which reads data from a binary stream.
	 * @param dataStream Stream to read data from.
	 */
	Stream(BinaryStream* dataStream);

	/**
	 * Creates a stream which reads data from a file.
	 * @param filename Filename of file to read data from.
	 */
	Stream(const char* filename);

	Stream(const Stream&);
	~Stream();

	/**
	 * Creates a stream that reads raw (16-bit short) PCM samples from a
	 * binary stream.
	 *
	 * @param dataStream Binary stream to read PCM data from.
	 * @param channels Number of channels in the PCM data.
	 * @param freq Sample rate (frequency) of the PCM data.
	 * @returns A stream utilizing dataStream.
	 */
	static Stream fromRaw(BinaryStream* dataStream, int channels, int freq);

	/**
	 * Returns raw PCM data in the stream's format containing the remaining
	 * audio processed off of the data stream.
	 *
	 * @returns A buffer containing 16-bit PCM data.
	 * @remarks This is a blocking function and may take time and CPU time
	 * in order to process and decode data.
	 * @warning This function may block indefinitely if the stream does not
	 * have an end condition.
	 */
	Buffer toRaw();

	/**
	 * Sets the destination format of the stream (i.e. the format returned by
	 * a read call.)
	 *
	 * @param numChannels Number of channels to output.
	 * @param sampleRate Sample rate to output to.
	 * @returns True if successful, false otherwise.
	 *
	 * @remarks Note that choosing a sample rate differing from that of the
	 * original data will cause resampling, which may slightly alter the
	 * behavior of the stream.
	 * @remarks Channel downmixing and upmixing is not implemented in any
	 * meaningful way for now.
	 */
	bool setFormat(int numChannels, int sampleRate);

	/**
	 * Reads at most bufferFrames frames into the buffer.
	 *
	 * @param buffer Buffer to store data in.
	 * @param bufferFrames Max frames to read, and after call, frames read.
	 *
	 * @remarks A single buffer frame contains a sample from each channel.
	 * This means the number of bytes is 2 * bufferFrames * numChannels.
	 */
	void read(float* buffer, int& bufferFrames);

	/**
	 * Resets the stream.
	 */
	void reset();

	/**
	 * Stops the stream.
	 */
	void stop();

	/**
	 * Pauses the stream.
	 */
	void pause();

	/**
	 * Resumes the stream.
	 */
	void resume();

	/**
	 * Determines whether or not the stream is over. Please note that atEnd
	 * will only signal for end-of-stream conditions, never when data is
	 * temporarily unavailable.
	 *
	 * @returns true if you are at the end of a stream.
	 */
	bool atEnd() const;
	void signalEnd();
	void signalLoop();
	bool hasLooped() const;

	/**
	 * Determins whether or not the stream has an end-of-stream condition.
	 *
	 * This does not tell you if the end condition is meaningful, however.
	 *
	 * @returns true if the stream can reach an end.
	 */
	bool haveEnd() const;

	/**
	 * Determines whether or not the stream is currently in an error state.
	 * Generally speaking, the stream is useless after this flag is set to
	 * true. All calls to the stream are undefined.
	 *
	 * @returns true if there is an error condition.
	 */
	bool error() const;

	/**
	 * Determines the current sample rate of the output of this stream.
	 * This may vary from the input stream and can be modified with setFormat.
	 *
	 * @returns the stream sample rate in Hz.
	 */
	int sampleRate() const;

	/**
	 * Determins the number of channels this stream will output. This may vary
	 * from the input stream and can be modified with setFormat.
	 *
	 * @returns the number of channels.
	 */
	int numChannels() const;

	/**
	 * @brief setObserver Sets the observer for this stream.
	 * The observer is an object that is called back when certain stream events
	 * occur.
	 *
	 * @warning The observer *WILL* be called from the audio thread, therefore
	 * be aware that anything done in the callback will happen on another
	 * thread.
	 *
	 * @param observer The observer to use.
	 */
	void setObserver(StreamObserver* observer);
	void setVolume(float volume);
	float volume() const;
	bool isMusic() const;
	void identifyAsMusic();

	bool operator == (const alib::Stream& other) const { return p == other.p; }
	bool operator != (const alib::Stream& other) const { return p != other.p; }
};

class StreamObserver
{
	friend struct Stream::Priv;
public:
	virtual ~StreamObserver() { }
private:
	virtual void reachedEnd() = 0;
	virtual void reachedLoop() = 0;
};

}
