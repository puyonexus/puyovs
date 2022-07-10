#pragma once

#include "audiolib.h"

namespace alib {

class Resampler
{
    ALIB_DECLARE_PRIV;

public:
    Resampler(int inNumChannels, int inSampleRate, int outNumChannels, int outSampleRate);
    ~Resampler();

    void resample(float *in, int *in_len, float *out, int *out_len);
    bool error();
};

}
