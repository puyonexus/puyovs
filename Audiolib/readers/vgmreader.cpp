#ifdef ALIB_VGMSTREAM_SUPPORT

#include "vgmreader.h"
#include "buffer.h"

extern "C" {
#include <../VgmStream/src/vgmstream.h>
#include <../VgmStream/src/streamfile.h>
}
#include <cstring>
#include <cmath>

namespace alib {

struct ALibStreamFile;
static ALibStreamFile *new_alib_streamfile(BinaryStream *alibStream);
static size_t alib_streamfile_read(STREAMFILE *sf, uint8_t *dest, off_t offset, size_t length);
static size_t alib_streamfile_get_size(STREAMFILE* sf);
static off_t alib_streamfile_get_offset(STREAMFILE* sf);
static void alib_streamfile_get_name(STREAMFILE* sf,char *buffer,size_t length);
static void alib_streamfile_get_realname(STREAMFILE* sf,char *buffer,size_t length);
static STREAMFILE *alib_streamfile_open(STREAMFILE* sf, const char * const filename, size_t buffersize);
static void alib_streamfile_close(STREAMFILE* sf);
static void alib_streamfile_reset(STREAMFILE* sf);

struct VgmReader::Priv
{
	VGMSTREAM* vgmstream = nullptr;
	ALibStreamFile* stmfile = nullptr;
	int freq = 0, channels = 0, length = 0, position = 0;
	int playposition = 0;

	Priv(BinaryStream* stream)
	{
		stream->rewind();
		stmfile = new_alib_streamfile(stream);
		vgmstream = init_vgmstream_from_STREAMFILE(reinterpret_cast<STREAMFILE*>(stmfile));

		if (!vgmstream)
			return;

		if (vgmstream->channels <= 0)
		{
			close_vgmstream(vgmstream);
			vgmstream = nullptr;
			return;
		}

		freq = vgmstream->sample_rate;
		channels = vgmstream->channels;
		length = get_vgmstream_play_samples(0, 0, 0, vgmstream);
		position = 0;
		playposition = 0;
	}

	~Priv()
	{
		close_vgmstream(vgmstream);
	}
};

VgmReader::VgmReader(Buffer& buffer)
	: p(new Priv(new MemoryStream(buffer)))
{
}

VgmReader::VgmReader(BinaryStream* stream)
	: p(new Priv(stream))
{
}

VgmReader::~VgmReader()
{
	delete p;
}

void VgmReader::read(float* buffer, int& bufferFrames)
{
	int32_t finalLength = bufferFrames;
	if (!p->vgmstream->loop_flag)
	{
		finalLength = std::min(p->length - p->position, bufferFrames);
	}

	// Rendering, -> 32-bit float
	auto sbuffer = static_cast<short*>(malloc(bufferFrames * p->channels * sizeof(short)));
	render_vgmstream(sbuffer, finalLength, p->vgmstream);

	for (int i = 0; i < finalLength * p->channels; ++i)
	{
		buffer[i] = static_cast<float>(sbuffer[i]) / 32768.f;
	}

	free(sbuffer);

	// Done.
	p->position += finalLength;
	p->playposition += std::min(p->length - p->position, bufferFrames);
	bufferFrames = finalLength;
}

void VgmReader::reset()
{
	p->position = 0;
	p->playposition = 0;
	reset_vgmstream(p->vgmstream);
	alib_streamfile_reset(reinterpret_cast<STREAMFILE*>(p->stmfile));
}

bool VgmReader::atEnd() const
{
	if (p->vgmstream->loop_flag)
	{
		return false;
	}

	return p->position >= p->length;
}

bool VgmReader::haveEnd() const
{
	return !p->vgmstream->loop_flag;
}

bool VgmReader::error() const
{
	return !p->vgmstream;
}

int VgmReader::numChannels() const
{
	return p->channels;
}

int VgmReader::sampleRate() const
{
	return p->freq;
}

bool VgmReader::hasLooped()
{
	return p->playposition >= p->length;
}

struct ALibStreamFile
{
	STREAMFILE sf;
	BinaryStream* alibStream;
	off_t offset;
	char name[260];
	char realname[260];
};

#ifdef _WIN32
#include <stdlib.h>
#include <cmath>

using namespace std;
#else
#include <libgen.h>
#endif

static ALibStreamFile* new_alib_streamfile(BinaryStream* alibStream)
{
	auto streamfile = static_cast<ALibStreamFile*>(malloc(sizeof(ALibStreamFile)));

	if (!streamfile)
	{
		return nullptr;
	}

	memset(streamfile, 0, sizeof(ALibStreamFile));

	streamfile->sf.read = alib_streamfile_read;
	streamfile->sf.get_size = alib_streamfile_get_size;
	streamfile->sf.get_offset = alib_streamfile_get_offset;
	streamfile->sf.get_name = alib_streamfile_get_name;
	streamfile->sf.get_realname = alib_streamfile_get_realname;
	streamfile->sf.open = alib_streamfile_open;
	streamfile->sf.close = alib_streamfile_close;

	streamfile->alibStream = alibStream;
	streamfile->offset = 0;

	strncpy(streamfile->name, alibStream->url().c_str(), sizeof(streamfile->name));
	streamfile->name[sizeof(streamfile->name) - 1] = '\0';

#ifdef _WIN32
	char fn_part[_MAX_FNAME] = { 0 };
	char ext_part[_MAX_EXT] = { 0 };

	_splitpath(alibStream->url().c_str(), nullptr, nullptr, fn_part, ext_part);
	strncpy(streamfile->realname, fn_part, min(_MAX_FNAME, 260));
	strncat(streamfile->realname, ext_part, 260 - strlen(streamfile->realname));
#else
	strncpy(streamfile->realname, basename(streamfile->name), sizeof(streamfile->realname));
#endif
	streamfile->realname[sizeof(streamfile->realname) - 1] = '\0';

	return streamfile;
}

static size_t alib_streamfile_read(STREAMFILE* sf, uint8_t* dest, off_t offset, size_t length)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);

	if (streamfile->offset != offset)
	{
		streamfile->alibStream->seek(offset, BinaryStream::Beginning);
		streamfile->offset = offset;
	}

	size_t sz = streamfile->alibStream->read(dest, length);
	streamfile->offset += static_cast<off_t>(sz);

	return sz;
}

static size_t alib_streamfile_get_size(STREAMFILE* sf)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);
	return streamfile->alibStream->size();
}

static off_t alib_streamfile_get_offset(STREAMFILE* sf)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);
	return streamfile->offset;
}

static void alib_streamfile_get_name(STREAMFILE* sf, char* buffer, size_t length)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);
	strncpy(buffer, streamfile->name, length);
	buffer[length - 1] = '\0';
}

static void alib_streamfile_get_realname(STREAMFILE* sf, char* buffer, size_t length)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);
	strncpy(buffer, streamfile->realname, length);
	buffer[length - 1] = '\0';
}

static STREAMFILE* alib_streamfile_open(STREAMFILE*/* streamfile */, const char* const filename, size_t /* buffersize */)
{
	if (!filename || !filename[0])
		return nullptr;

	return &new_alib_streamfile(BinaryStream::openUrl(filename))->sf;
}

static void alib_streamfile_close(STREAMFILE* sf)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);

	delete streamfile->alibStream;

	free(streamfile);
}

static void alib_streamfile_reset(STREAMFILE* sf)
{
	auto streamfile = reinterpret_cast<ALibStreamFile*>(sf);
	streamfile->offset = 0;
	streamfile->alibStream->rewind();
}

}

#endif
