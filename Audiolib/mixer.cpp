#define TEST
#include "mixer.h"
#include <list>
#include <cstring>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

namespace alib {

Mixer::~Mixer()
{
}

struct SoftwareMixer::Priv {
    struct PlayingStream {
        PlayingStream(Stream &st) : stm(st) { }
        alib::Stream stm;

        bool done()
        {
            if(!stm.atEnd())
                return false;

            return true;
        }
        bool looped()
        {
            return stm.hasLooped();
        }
    };

    int channels, rate, frameSize, maxFrames;
    std::list<PlayingStream> streams;
    static const int scratchBufferLen = 64 * 1024;
    static const int sampleSize = sizeof(float);
    float *scratchBuffer;
    float masterVolume;
    float musicVolume,soundVolume;

    Priv() : channels(2), rate(44100),masterVolume(1.0f),musicVolume(1.0f),soundVolume(1.0f)
    {
        setFormat(rate, channels);
        scratchBuffer = new float[scratchBufferLen];
    }

    ~Priv() { delete [] scratchBuffer; }

    bool setFormat(int r, int c)
    {
        if(r > 1000) rate = r;
        if(c > 0) channels = c;

        // Truncate the overbuffers, if any exist, since they are now invalid.
        for(std::list<Priv::PlayingStream>::iterator it = streams.begin();
            it != streams.end(); ++it)
        {
            PlayingStream &e = *it;
        }

        frameSize = sampleSize * channels;
        maxFrames = scratchBufferLen / frameSize;

        return true;
    }

    void read(float *buffer, int &bufferFrames)
    {
        int bufferSamples = bufferFrames * channels;
        int scratchBufferSamples = (bufferFrames) * channels;
        int bufPtr = 0;

        memset(buffer, 0, bufferFrames * frameSize);

        for(std::list<Priv::PlayingStream>::iterator it = streams.begin();
            it != streams.end(); ++it)
        {
            PlayingStream &e = *it;

            bufPtr = 0;
            memset(scratchBuffer, 0, scratchBufferSamples * sampleSize);

            int requestFrames = (scratchBufferSamples - bufPtr) / channels;

            e.stm.read(scratchBuffer + bufPtr, requestFrames);
            bufPtr += requestFrames * channels;

            int mixLen = std::min(bufPtr, bufferSamples);
            // Mixing
            for(int i = 0; i < mixLen; ++i)
                buffer[i] = (buffer[i] + scratchBuffer[i] * e.stm.volume() * (e.stm.isMusic() ? musicVolume : soundVolume) )
                        *masterVolume;
        }

        std::list<Priv::PlayingStream>::iterator it = streams.begin();

        while(it != streams.end())
        {
            if ((*it).looped())
            {
                (*it).stm.signalLoop();
            }
            if((*it).done())
            {
                (*it).stm.signalEnd();
                it = streams.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    bool play(Stream &stream)
    {
        std::list<Priv::PlayingStream>::iterator it = streams.begin();
        while(it != streams.end())
        {
            if((*it).stm == stream)
                it = streams.erase(it);
            else
                ++it;
        }
        stream.reset();

        if(stream.setFormat(channels, rate) && !stream.error())
        {
            streams.push_back(Priv::PlayingStream(stream));

            return true;
        }
        else return false;
    }

    void setVolume(float volume)
    {
        masterVolume=volume;
    }
    void setMusicVolume(float volume)
    {
        musicVolume=volume;
    }
    void setSoundVolume(float volume)
    {
        soundVolume=volume;
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

bool SoftwareMixer::setFormat(int rate, int channels)
{
    return p->setFormat(rate, channels);
}

void SoftwareMixer::read(float *buffer, int &length)
{
    p->read(buffer, length);
}

bool SoftwareMixer::play(Stream stream)
{
    return p->play(stream);
}

int SoftwareMixer::sampleRate()
{
    return p->rate;
}

int SoftwareMixer::numChannels()
{
    return p->channels;
}

bool SoftwareMixer::error()
{
    return false;
}

void SoftwareMixer::setVolume(float volume)
{
    p->setVolume(volume);
}

void SoftwareMixer::setSoundVolume(float volume)
{
    p->setSoundVolume(volume);
}

void SoftwareMixer::setMusicVolume(float volume)
{
    p->setMusicVolume(volume);
}

}
