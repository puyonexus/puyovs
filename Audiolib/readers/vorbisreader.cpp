#include "vorbisreader.h"
#include "resampler.h"
#include "minivorbis.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

namespace alib {

#ifdef ALIB_OGG_SUPPORT
struct VorbisReader::Priv
{
	Priv(BinaryStream* strm) : strm(strm)
	{
		int err = VORBIS__no_error;

		setVorbis(stb_vorbis_open_file(strm, 0, &err, nullptr));

		if (err != VORBIS__no_error)
		{
			ALIB_RETURN_ERROR("Error loading vorbis stream [%u]\n", err);
		}

		error = false;
	}

	~Priv()
	{
		stb_vorbis_close(vorbis);

		delete strm;
	}

	stb_vorbis* vorbis = nullptr;
	stb_vorbis_info info;
	Resampler* resampler = nullptr;
	BinaryStream* strm = nullptr;
	int loopStart = 0, loopEnd, loopLen = 0, cursor = 0;

	int sampleRate = 0, numChannels = 0, numSamples = 0;
	bool needConversion = false, error = true;
	Buffer buffer;

	void setVorbis(stb_vorbis* v)
	{
		if (!v)
		{
			return;
		}
		
		stb_vorbis_close(vorbis);

		vorbis = v;
		info = stb_vorbis_get_info(v);

		loopStart = loopLen = cursor = 0;

		for (const auto name : { "LOOPSTART", "loop_start", "LOOP_START", "LoopStart" })
		{
			const char *field = stb_vorbis_get_comment_by_field(vorbis, name, nullptr);
			if (field)
			{
				loopStart = atoi(field);
				break;
			}
		}

		for (const auto name : { "LOOPLENGTH", "loop_length", "LOOP_LENGTH", "LoopLength" })
		{
			const char* field = stb_vorbis_get_comment_by_field(vorbis, name, nullptr);
			if (field)
			{
				loopLen = atoi(field);
				break;
			}
		}

		numChannels = info.channels;
		numSamples = strm->seekable() ? stb_vorbis_stream_length_in_samples(v) : 0;
		sampleRate = info.sample_rate;

		if (loopLen > 0)
		{
			loopEnd = loopStart + loopLen;
		}
		else
		{
			loopEnd = numSamples;
		}

		if (loopEnd < loopStart)
		{
			loopEnd = loopStart = loopLen = 0;
		}
	}

	void read(float* buffer, int& length)
	{
		int firstLen = length, secondLen = 0, wantLen = length,
			wantEnd = loopEnd > 0 ? std::min(loopEnd, numSamples) : numSamples;

		if (error || !vorbis)
		{
			length = 0;
			return;
		}

		if (wantEnd > 0 && length + cursor >= wantEnd)
		{
			firstLen = wantEnd - cursor;
		}

		firstLen = stb_vorbis_get_samples_float_interleaved(vorbis, numChannels, buffer, firstLen * numChannels);
		cursor += firstLen;
		buffer += firstLen * numChannels;
		wantLen -= firstLen;

		if (loopLen > 0 && wantLen > 0 && wantEnd > 0 && cursor >= wantEnd)
		{
			reset();
			seekForward(loopStart);

			secondLen = wantLen;
			read(buffer, secondLen);
		}

		length = firstLen + secondLen;
	}

	void seekForward(int samples)
	{
		if (samples < 1)
		{
			return;
		}

		cursor += samples;
		stb_vorbis_seek_forward(vorbis, samples);
	}

	void reset()
	{
		cursor = 0;
		stb_vorbis_seek_start(vorbis);
	}

	bool atEnd()
	{
		return numSamples > 0 ? cursor >= numSamples : false;
	}
};

VorbisReader::VorbisReader(BinaryStream* stream)
	: p(new Priv(stream))
{
}

VorbisReader::~VorbisReader()
{
	delete p;
}

void VorbisReader::read(float* buffer, int& bufferFrames)
{
	p->read(buffer, bufferFrames);
}

void VorbisReader::reset()
{
	p->reset();
}

bool VorbisReader::atEnd() const
{
	return p->atEnd();
}

bool VorbisReader::haveEnd() const
{
	return p->numSamples > 0;
}

bool VorbisReader::error() const
{
	return p->error;
}

int VorbisReader::numChannels() const
{
	return p->numChannels;
}

int VorbisReader::sampleRate() const
{
	return p->sampleRate;
}

bool VorbisReader::hasLooped()
{
	return false;
}
#endif

}
