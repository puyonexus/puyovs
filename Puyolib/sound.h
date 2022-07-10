#pragma once

#include <string>

namespace ppvs
{
struct gameData;
class game;
class fsound;

class sound
{
    public:
        sound();
        virtual ~sound();

        void Play(gameData *g);
        void Stop(gameData *g);
        void SetBuffer(fsound *sound);
        void Unload();
        ppvs::fsound *GetBuffer();

    private:
        gameData *gamedata;
        fsound *m_sound;
};

}
