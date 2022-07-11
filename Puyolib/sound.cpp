#include "sound.h"
#include "global.h"
#include "game.h"


namespace ppvs
{
sound::sound()
    : m_sound(nullptr)
{
    //ctor
}

sound::~sound()
{
}

void sound::Play(gameData* global)
{
    if (global== nullptr || !global->playSounds || !global->windowFocus)
        return;


    if (m_sound)
        m_sound->play();
}

void sound::Stop(gameData *)
{
    if (m_sound)
        m_sound->stop();
}

void sound::SetBuffer(fsound *sound)
{
    m_sound = sound;
}

fsound* sound::GetBuffer()
{
    return m_sound;
}

void sound::Unload()
{
    delete m_sound;
    m_sound = nullptr;
}

}
