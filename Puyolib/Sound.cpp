#include "Sound.h"
#include "global.h"
#include "Game.h"

namespace ppvs
{

Sound::Sound()
	: m_sound(nullptr)
{
}

Sound::~Sound() = default;

void Sound::play(GameData* global)
{
	if (global == nullptr || !global->playSounds || !global->windowFocus)
		return;

	if (m_sound)
		m_sound->play();
}

void Sound::stop(GameData*)
{
	if (m_sound)
		m_sound->stop();
}

void Sound::setBuffer(FeSound* sound)
{
	m_sound = sound;
}

FeSound* Sound::getBuffer()
{
	return m_sound;
}

void Sound::unload()
{
	delete m_sound;
	m_sound = nullptr;
}

}
