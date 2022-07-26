#pragma once

namespace ppvs {

struct GameData;
class Game;
class FeSound;

class Sound final {
public:
	Sound();
	~Sound();

	void play(const GameData* data);
	void stop(GameData* data);
	void setBuffer(FeSound* sound);
	void unload();
	[[nodiscard]] ppvs::FeSound* getBuffer() const;

private:
	FeSound* m_sound = nullptr;
};

}
