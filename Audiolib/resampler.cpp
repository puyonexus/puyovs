#include "resampler.h"
#include "speex/speex_resampler.h"

namespace alib {

struct Resampler::Priv
{
	Priv(int inNumChannels, int outNumChannels) : error(false), needReframe(inNumChannels != outNumChannels),
		inNumChannels(inNumChannels), outNumChannels(outNumChannels),
		resampler(nullptr)
	{
	}

	~Priv()
	{
		if (resampler)
		{
			speex_resampler_destroy(resampler);
		}
	}

	bool error, needReframe;
	int inNumChannels, outNumChannels;
	SpeexResamplerState* resampler;

	void resample(float* in, int* in_len, float* out, int* out_len) const
	{
		float* in_adj = in;

		if (needReframe)
		{
			const int looplen = (*in_len);
			in_adj = new float[(*in_len) * outNumChannels];

			for (int i = 0; i < looplen; ++i)
				for (int j = 0; j < outNumChannels; ++j)
					in_adj[(i * outNumChannels) + j] = in[(i * inNumChannels) + (j % inNumChannels)];
		}

		// Load length
		spx_uint32_t u32_in_len = (*in_len), u32_out_len = (*out_len);

		// Do actual resample (using speex resampler)
		speex_resampler_process_interleaved_float(resampler, in_adj, &u32_in_len, out, &u32_out_len);

		// Save length
		*in_len = u32_in_len;
		*out_len = u32_out_len;

		if (needReframe) delete[] in_adj;
	}
};

Resampler::Resampler(int inNumChannels, int inSampleRate, int outNumChannels, int outSampleRate)
	: p(new Priv(inNumChannels, outNumChannels))
{
	int err = RESAMPLER_ERR_SUCCESS;
	p->resampler = speex_resampler_init(outNumChannels, inSampleRate, outSampleRate, 0, &err);

	if (err != RESAMPLER_ERR_SUCCESS)
	{
		p->error = true;
	}
}

Resampler::~Resampler()
{
	delete p;
}

void Resampler::resample(float* in, int* in_len, float* out, int* out_len) const
{
	p->resample(in, in_len, out, out_len);
}

bool Resampler::error() const
{
	return p->error;
}

}
