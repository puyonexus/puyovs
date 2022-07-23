#include "stream.h"
#include "samplereader.h"
#include "resampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "readers/vorbisreader.h"
#include "readers/vgmreader.h"
#include "readers/pcmreader.h"

namespace alib {

struct Stream::Priv
{
	Priv() : inNumChannels(2), inSampleRate(44100),
		outNumChannels(2), outSampleRate(44100),
		needConversion(false), error(false), paused(false),
		volume(1.0f), music(false),
		observer(0), reader(0), resampler(0) { }

	~Priv() { delete resampler; delete reader; }

	int inNumChannels, inSampleRate;
	int outNumChannels, outSampleRate;
	bool needConversion, error, paused;
	float volume; bool music;
	StreamObserver* observer;
	SampleReader* reader;
	Resampler* resampler;
	Buffer resamplingBuffer;

	void loadFromStream(BinaryStream* dataStream)
	{
		if (!dataStream || dataStream->error())
		{
			delete dataStream;
			error = true;
			return;
		}

		ALIB_VGM_DETECT(reader, dataStream, dataStream);
		ALIB_OGG_DETECT(reader, dataStream, dataStream);

		if (reader && !reader->error())
		{
			return;
		}

		error = true;
	}

	void loadFromBuffer(Buffer& buffer)
	{
		MemoryStream* dataStream = new MemoryStream(buffer);
		loadFromStream(dataStream);

		if (error)
		{
			delete dataStream;
		}
	}

	bool setFormat(int numChannels, int sampleRate)
	{
		if (!reader || error)
		{
			return false;
		}

		if (numChannels > 0)
		{
			outNumChannels = numChannels;
		}

		if (sampleRate > 1000)
		{
			outSampleRate = sampleRate;
		}

		inNumChannels = reader->numChannels();
		inSampleRate = reader->sampleRate();

		needConversion = 
			inNumChannels != outNumChannels ||
			inSampleRate != outSampleRate;

		delete resampler;

		if (needConversion)
		{
			resampler = new Resampler(inNumChannels, inSampleRate,
				outNumChannels, outSampleRate);
		}

		return true;
	}

	void read(float* buffer, int& length)
	{
		if (!reader || error || reader->atEnd())
		{
			length = 0;
			return;
		}

		// If we're paused, just return an empty frame.
		if (paused)
		{
			const int numSamples = length * outNumChannels;
			for (int i = 0; i < numSamples; ++i)
			{
				buffer[i] = 0;
			}
			return;
		}

		if (needConversion && resampler)
		{
			if (inSampleRate == outSampleRate)
			{
				float* bufferIn = new float[length * inNumChannels];
				reader->read(bufferIn, length);

				for (int i = 0; i < length; ++i)
					for (int j = 0; j < outNumChannels; ++j)
						buffer[(i * outNumChannels) + j] = bufferIn[(i * inNumChannels) + (j % inNumChannels)];

				delete[] bufferIn;
			}
			else
			{
				int fullLengthIn = ((long long)length * inSampleRate) / outSampleRate;
				int neededLengthIn = fullLengthIn;
				float* bufferIn = new float[fullLengthIn * inNumChannels];
				memset(bufferIn, 0, sizeof(float) * fullLengthIn * inNumChannels);
				memset(buffer, 0, sizeof(float) * length * outNumChannels);

				int recalled = resamplingBuffer.read(bufferIn, neededLengthIn * inNumChannels * sizeof(float), 0) / (sizeof(float));
				neededLengthIn -= recalled / inNumChannels;
				int lengthInRecv = neededLengthIn;
				reader->read(bufferIn + recalled, lengthInRecv);

				resamplingBuffer.append((bufferIn + recalled), neededLengthIn * inNumChannels * sizeof(float));
				resampler->resample(bufferIn, &fullLengthIn, buffer, &length);
				resamplingBuffer.removeFront(fullLengthIn * inNumChannels * sizeof(float));

				delete[] bufferIn;
			}
		}
		else
		{
			reader->read(buffer, length);
		}

	}

	void reset()
	{
		if (reader && !error)
			reader->reset();
	}

	void stop()
	{
		reader = 0;
		error = true;
		observer = 0;
	}

	void pause()
	{
		paused = true;
	}

	void resume()
	{
		paused = false;
	}

	void signalEnd()
	{
		if (observer) observer->reachedEnd();
	}

	void signalLoop()
	{
		if (observer) observer->reachedLoop();
	}
};

Stream::Stream()
	: p(new Priv)
{
	p->error = true;
}

Stream::Stream(Buffer& buffer)
	: p(new Priv)
{
	p->loadFromBuffer(buffer);
}

Stream::Stream(BinaryStream* dataStream)
	: p(new Priv)
{
	p->loadFromStream(dataStream);
}

Stream::Stream(const char* filename)
	: p(new Priv)
{
	p->loadFromStream(new alib::FileStream(filename, "rb"));
}

Stream::Stream(const Stream& other)
	: p(other.p)
{
}

Stream::~Stream() = default;

Stream Stream::fromRaw(BinaryStream* dataStream, int channels, int freq)
{
	Stream stream = Stream();

	stream.p->error = false;
	stream.p->reader = new PCMReader(dataStream, channels, freq);

	return stream;
}

Buffer Stream::toRaw()
{
	const int frameSize = numChannels() * sizeof(float);
	float* buffer = static_cast<float*>(malloc(4096 * frameSize));
	Buffer finalBuffer;

	while (!atEnd())
	{
		int frames = 4096;
		read(buffer, frames);

		if (frames > 0)
		{
			finalBuffer.append(buffer, frames * frameSize);
		}
	}

	free(buffer);
	return finalBuffer;
}

bool Stream::setFormat(int numChannels, int sampleRate)
{
	return p->setFormat(numChannels, sampleRate);
}

void Stream::read(float* buffer, int& bufferFrames)
{
	p->read(buffer, bufferFrames);
}

void Stream::reset()
{
	p->reset();
}

void Stream::stop()
{
	p->stop();
}

void Stream::pause()
{
	p->pause();
}

void Stream::resume()
{
	p->resume();
}

bool Stream::atEnd() const
{
	if (p->reader && !p->error)
		return p->reader->atEnd();
	else
		return true;
}

void Stream::signalEnd()
{
	p->signalEnd();
}

void Stream::signalLoop()
{
	p->signalLoop();
}

bool Stream::hasLooped() const
{
	if (p->reader && !p->error)
		return p->reader->hasLooped();
	else
		return false;
}

bool Stream::haveEnd() const
{
	if (p->reader && !p->error)
		return p->reader->haveEnd();
	else
		return true;
}

bool Stream::error() const
{
	return p->error || p->reader->error();
}

int Stream::sampleRate() const
{
	return p->outSampleRate;
}

int Stream::numChannels() const
{
	return p->outNumChannels;
}

void Stream::setObserver(StreamObserver* observer)
{
	p->observer = observer;
}

float Stream::volume() const
{
	return p->volume;
}

bool Stream::isMusic() const
{
	return p->music;
}

void Stream::identifyAsMusic()
{
	p->music = true;
	p->volume = 0;
}

void Stream::setVolume(float volume)
{
	p->volume = volume;
}

}
