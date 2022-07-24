#pragma once

#include "global.h"
#include "Sprite.h"
#include "DropPattern.h"

namespace ppvs
{

class Game;

class CharacterSelect
{
public:
	CharacterSelect(Game*);
	virtual ~CharacterSelect();

	void draw();
	void prepare();
	void play();
	void end();

	void setCharacter(int playernum, int selection, bool choice);

private:
	void setDropset(int x, int y, int pl);
	int findCurrentCharacter(int i);

	int timer = 0;
	Game* currentgame = nullptr;
	GameData* data = nullptr;
	PuyoCharacter order[24] = {
		ARLE, SCHEZO, RULUE, DRACO, AMITIE, RAFFINE, SIG, RIDER,
		WITCH, SATAN, SUKETOUDARA, CARBUNCLE, ACCORD, KLUG, DONGURIGAERU, OCEAN_PRINCE,
		RINGO, MAGURO, RISUKUMA, ECOLO, FELI, LEMRES, OSHARE_BONES, YU_REI,
	};
	Sprite background = {};
	Sprite holder[24] = {};
	Sprite charSprite[24] = {};
	FeImage* backgroundImage = nullptr;

	// Player stuff
	Sprite* selectSprite = nullptr;
	Sprite* selectedCharacter = nullptr;
	Sprite* dropset = nullptr;
	Sprite* name = nullptr;
	Sprite* nameHolder = nullptr;
	Sprite* nameHolderNumber = nullptr;
	Sprite* playernumber = nullptr;
	int* sel = nullptr;
	bool* madeChoice = nullptr;
	float scale = 1.f;
	int Nplayers = 0;
	bool firstStart = true;
};

}
