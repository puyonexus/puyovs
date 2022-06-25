#ifndef MIXER_H
#define MIXER_H

#include "audiolib.h"
#include "stream.h"

namespace alib {

class Mixer
{
public:
    virtual ~Mixer();

    virtual bool setFormat(int rate, int channels) = 0;
    virtual void read(float *buffer, int &length) = 0;
    virtual bool play(Stream stream) = 0;

    virtual int sampleRate() = 0;
    virtual int numChannels() = 0;
    virtual bool error() = 0;
};

/**
 * Mixer; mixes several streams simultaneously. Introduces ~1024 samples of lag.
 */
class SoftwareMixer : public Mixer, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    SoftwareMixer();
    ~SoftwareMixer();

    bool setFormat(int rate, int channels);
    void read(float *buffer, int &length);
    bool play(Stream stream);

    int sampleRate();
    int numChannels();
    bool error();
    void setVolume(float volume);
    void setSoundVolume(float volume);
    void setMusicVolume(float volume);
};

}

#endif // MIXER_H
