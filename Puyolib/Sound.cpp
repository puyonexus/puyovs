#include "Sound.h"
#include "Game.h"
#include "global.h"

namespace ppvs {

Sound::Sound() = default;

Sound::~Sound() = default;

void Sound::play(const GameData* data)
{
	if (data == nullptr || !data->playSounds || !data->windowFocus) {
		return;
	}

	if (m_sound) {
		m_sound->play();
	}
}

void Sound::stop(GameData*)
{
	if (m_sound) {
		m_sound->stop();
	}
}

void Sound::setBuffer(FeSound* sound)
{
	m_sound = sound;
}

FeSound* Sound::getBuffer() const
{
	return m_sound;
}

void Sound::unload()
{
	delete m_sound;
	m_sound = nullptr;
}

}
