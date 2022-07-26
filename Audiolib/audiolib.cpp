#include "audiolib.h"
#include "fast_mutex.h"
#include "mixer.h"
#include <SDL.h>

using namespace tthread;

namespace alib {

Device* device = nullptr;

Device* open()
{
	if (!device) {
		device = new Device;
	}
	return device;
}

void close()
{
	delete device;
}

struct Device::Priv {
	int device;
	SoftwareMixer mixer;
	fast_mutex mixmutex;
	bool quitsignal;

	Priv()
		: device(0)
	{
		quitsignal = false;
	}

	static void callback(Priv* p, Uint8* samples, int len)
	{
		p->mixmutex.lock();
		if (p->quitsignal)
			return; // Inside mutex for a reason
		len /= (sizeof(float) * p->mixer.numChannels());
		p->mixer.read((float*)samples, len);
		p->mixmutex.unlock();
	}
};

Device::Device()
	: p(new Priv)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		ALIB_ERROR("Failed to initialize SDL: %s\n", SDL_GetError());
	}

	SDL_AudioSpec desire = {
		44100, AUDIO_F32, 2, 0, 4096, 0, 0,
		reinterpret_cast<SDL_AudioCallback>(&Priv::callback), p
	},
				  obtained;

	SDL_OpenAudio(&desire, &obtained);
	p->mixer.setFormat(obtained.freq, obtained.channels);
	SDL_PauseAudio(0);
}

Device::~Device()
{
	p->mixmutex.lock();
	p->quitsignal = true;

	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	p->mixmutex.unlock();

	delete p;
}

bool Device::play(const Stream& stm)
{
	p->mixmutex.lock();
	bool playing = p->mixer.play(stm);
	p->mixmutex.unlock();
	return playing;
}

void Device::setVolume(float volume)
{
	p->mixmutex.lock();
	p->mixer.setVolume(volume);
	p->mixmutex.unlock();
}

void Device::setSoundVolume(float volume)
{
	p->mixmutex.lock();
	p->mixer.setSoundVolume(volume);
	p->mixmutex.unlock();
}

void Device::setMusicVolume(float volume)
{
	p->mixmutex.lock();
	p->mixer.setMusicVolume(volume);
	p->mixmutex.unlock();
}

}
