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
    Priv(BinaryStream *strm) : vorbis(0), resampler(0),
        sampleRate(0), numChannels(0), numSamples(0),
        needConversion(false), error(true), strm(strm),
        loopStart(0), loopLen(0), cursor(0)
    {
        int err = VORBIS__no_error;

        setVorbis(stb_vorbis_open_file(strm, 0, &err, 0));

        if(err != VORBIS__no_error)
            ALIB_RETURN_ERROR("Error loading vorbis stream [%u]\n", err);
        else error = false;
    }

    ~Priv()
    {
        if(vorbis)
            stb_vorbis_close(vorbis);

        delete strm;
    }

    stb_vorbis *vorbis;
    stb_vorbis_info info;
    Resampler *resampler;
    BinaryStream *strm;
    int loopStart, loopEnd, loopLen, cursor;

    int sampleRate, numChannels, numSamples;
    bool needConversion, error;
    Buffer buffer;

    void setVorbis(stb_vorbis *v)
    {
        if(!v)
            return;

        if(vorbis)
            stb_vorbis_close(vorbis);

        vorbis = v;
        info = stb_vorbis_get_info(v);

        loopStart = loopLen = cursor = 0;

        const char *loopStartNames[] = { "LOOPSTART", "loop_start", "LOOP_START", "LoopStart", 0 };
        const char *loopLengthNames[] = { "LOOPLENGTH", "loop_length", "LOOP_LENGTH", "LoopLength", 0 };
        const char **i = 0;
        const char *field;

        for(i = loopStartNames; *i != 0; ++i )
        {
            field = stb_vorbis_get_comment_by_field(vorbis, *i, 0);

            if(field != 0)
                loopStart = atoi(field);
        }

        for(i = loopLengthNames; *i != 0; ++i )
        {
            field = stb_vorbis_get_comment_by_field(vorbis, *i, 0);

            if(field != 0)
                loopLen = atoi(field);
        }

        numChannels = info.channels;
        numSamples = strm->seekable() ? stb_vorbis_stream_length_in_samples(v) : 0;
        sampleRate = info.sample_rate;

        if(loopLen > 0)
            loopEnd = loopStart + loopLen;
        else
            loopEnd = numSamples;

        if(loopEnd < loopStart)
            loopEnd = loopStart = loopLen = 0;
    }

    void read(float *buffer, int &length)
    {
        int firstLen = length, secondLen = 0, wantLen = length,
            wantEnd = loopEnd > 0 ? std::min(loopEnd, numSamples) : numSamples;

        if(error || !vorbis) { length = 0; return; }
        if(wantEnd > 0 && length + cursor >= wantEnd)
            firstLen = wantEnd - cursor;

        firstLen = stb_vorbis_get_samples_float_interleaved(vorbis, numChannels, buffer, firstLen * numChannels);
        cursor += firstLen;
        buffer += firstLen * numChannels;
        wantLen -= firstLen;

        if(loopLen > 0 && wantLen > 0 && wantEnd > 0 && cursor >= wantEnd)
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
        if(samples < 1)
            return;

        cursor += samples;
        stb_vorbis_seek_forward(vorbis, samples);

        /*float *sampleBuffer = new float[samples * numChannels];
        cursor += stb_vorbis_get_samples_float_interleaved(vorbis, numChannels, sampleBuffer, samples * numChannels);
        delete [] sampleBuffer;*/
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

VorbisReader::VorbisReader(BinaryStream *stream)
    : p(new Priv(stream))
{
}

VorbisReader::~VorbisReader()
{
    delete p;
}

void VorbisReader::read(float *buffer, int &bufferFrames)
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
