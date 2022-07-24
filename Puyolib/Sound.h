#pragma once

#include <string>

namespace ppvs
{

struct GameData;
class Game;
class FeSound;

class Sound
{
public:
	Sound();
	virtual ~Sound();

	void play(GameData* g);
	void stop(GameData* g);
	void setBuffer(FeSound* sound);
	void unload();
	ppvs::FeSound* getBuffer();

private:
	GameData* gamedata;
	FeSound* m_sound;
};

}
