#define TEST
#include "mixer.h"
#include <cstring>
#include <list>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

namespace alib {

Mixer::~Mixer() = default;

struct SoftwareMixer::Priv {
	struct PlayingStream {
		PlayingStream(const Stream& st)
			: stm(st)
		{
		}
		alib::Stream stm;

		bool done() const
		{
			return stm.atEnd();
		}

		bool looped() const
		{
			return stm.hasLooped();
		}
	};

	int channels, rate, frameSize, maxFrames;
	std::list<PlayingStream> streams;
	static constexpr int scratchBufferLen = 64 * 1024;
	static constexpr int sampleSize = sizeof(float);
	float* scratchBuffer;
	float masterVolume;
	float musicVolume, soundVolume;

	Priv()
		: channels(2)
		, rate(44100)
		, masterVolume(1.0f)
		, musicVolume(1.0f)
		, soundVolume(1.0f)
	{
		setFormat(rate, channels);
		scratchBuffer = new float[scratchBufferLen];
	}

	~Priv() { delete[] scratchBuffer; }

	bool setFormat(int r, int c)
	{
		if (r > 1000)
			rate = r;
		if (c > 0)
			channels = c;

		// Truncate the overbuffers, if any exist, since they are now invalid.
		for (auto& e : streams) {
			// Something was probably meant to be done here...
			// but what?!
		}

		frameSize = sampleSize * channels;
		maxFrames = scratchBufferLen / frameSize;

		return true;
	}

	void read(float* buffer, const int& bufferFrames)
	{
		const int bufferSamples = bufferFrames * channels;
		const int scratchBufferSamples = (bufferFrames)*channels;
		int bufPtr = 0;

		memset(buffer, 0, bufferFrames * frameSize);

		for (auto& e : streams) {
			bufPtr = 0;
			memset(scratchBuffer, 0, scratchBufferSamples * sampleSize);

			int requestFrames = (scratchBufferSamples - bufPtr) / channels;

			e.stm.read(scratchBuffer + bufPtr, requestFrames);
			bufPtr += requestFrames * channels;

			int mixLen = std::min(bufPtr, bufferSamples);

			// Mixing
			for (int i = 0; i < mixLen; ++i) {
				buffer[i] = (buffer[i] + scratchBuffer[i] * e.stm.volume() * (e.stm.isMusic() ? musicVolume : soundVolume))
					* masterVolume;
			}
		}

		auto it = streams.begin();

		while (it != streams.end()) {
			if ((*it).looped()) {
				(*it).stm.signalLoop();
			}
			if ((*it).done()) {
				(*it).stm.signalEnd();
				it = streams.erase(it);
			} else {
				++it;
			}
		}
	}

	bool play(Stream& stream)
	{
		auto it = streams.begin();
		while (it != streams.end()) {
			if ((*it).stm == stream)
				it = streams.erase(it);
			else
				++it;
		}
		stream.reset();

		if (stream.setFormat(channels, rate) && !stream.error()) {
			streams.emplace_back(stream);

			return true;
		}
		return false;
	}

	void setVolume(const float volume)
	{
		masterVolume = volume;
	}

	void setMusicVolume(const float volume)
	{
		musicVolume = volume;
	}

	void setSoundVolume(const float volume)
	{
		soundVolume = volume;
	}
};

SoftwareMixer::SoftwareMixer()
	: p(new Priv)
{
}

SoftwareMixer::~SoftwareMixer()
{
	delete p;
}

bool SoftwareMixer::setFormat(const int rate, const int channels)
{
	return p->setFormat(rate, channels);
}

void SoftwareMixer::read(float* buffer, int& length)
{
	p->read(buffer, length);
}

bool SoftwareMixer::play(Stream stream)
{
	return p->play(stream);
}

int SoftwareMixer::sampleRate() const
{
	return p->rate;
}

int SoftwareMixer::numChannels() const
{
	return p->channels;
}

bool SoftwareMixer::error() const
{
	return false;
}

void SoftwareMixer::setVolume(const float volume)
{
	p->setVolume(volume);
}

void SoftwareMixer::setSoundVolume(const float volume)
{
	p->setSoundVolume(volume);
}

void SoftwareMixer::setMusicVolume(float volume)
{
	p->setMusicVolume(volume);
}

}
