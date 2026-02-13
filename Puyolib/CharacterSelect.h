#pragma once

#include "Sprite.h"
#include "global.h"

namespace ppvs {

class Game;

class CharacterSelect final {
public:
	explicit CharacterSelect(Game*);
	~CharacterSelect();

	CharacterSelect(const CharacterSelect&) = delete;
	CharacterSelect& operator=(const CharacterSelect&) = delete;
	CharacterSelect(CharacterSelect&&) = delete;
	CharacterSelect& operator=(CharacterSelect&&) = delete;
	void hotRedraw();

	void draw();
	void prepare();
	void play();
	void end();

	void setCharacter(int playerNum, int selection, bool choice);

private:
	void setDropset(int x, int y, int pl);
	[[nodiscard]] int findCurrentCharacter(int i) const;

	int m_timer = 0;
	Game* m_currentGame = nullptr;
	GameData* m_data = nullptr;
	PuyoCharacter m_order[24] = {
		ARLE,
		SCHEZO,
		RULUE,
		DRACO,
		AMITIE,
		RAFFINE,
		SIG,
		RIDER,
		WITCH,
		SATAN,
		SUKETOUDARA,
		CARBUNCLE,
		ACCORD,
		KLUG,
		DONGURIGAERU,
		OCEAN_PRINCE,
		RINGO,
		MAGURO,
		RISUKUMA,
		ECOLO,
		FELI,
		LEMRES,
		OSHARE_BONES,
		YU_REI,
	};
	Sprite m_background = {};
	Sprite m_holder[24] = {};
	Sprite m_charSprite[24] = {};

	// Player stuff
	Sprite* m_selectSprite = nullptr;
	Sprite* m_selectedCharacter = nullptr;
	Sprite* m_dropSet = nullptr;
	Sprite* m_name = nullptr;
	Sprite* m_nameHolder = nullptr;
	Sprite* m_nameHolderNumber = nullptr;
	Sprite* m_playerNumber = nullptr;
	int* m_sel = nullptr;
	bool* m_madeChoice = nullptr;
	float m_scale = 1.f;
	int m_numPlayers = 0;
	bool m_firstStart = true;
};

}
