#pragma once

#include "global.h"
#include "sprite.h"
#include "dropPattern.h"

namespace ppvs
{

class game;

class characterSelect
{
public:
	characterSelect(game*);
	virtual ~characterSelect();

	void draw();
	void prepare();
	void play();
	void end();

	void setCharacter(int playernum, int selection, bool choice);

private:
	void setDropset(int x, int y, int pl);
	int findCurrentCharacter(int i);

	int timer = 0;
	game* currentgame = nullptr;
	gameData* data = nullptr;
	puyoCharacter order[24] = {
		ARLE, SCHEZO, RULUE, DRACO, AMITIE, RAFFINE, SIG, RIDER,
		WITCH, SATAN, SUKETOUDARA, CARBUNCLE, ACCORD, KLUG, DONGURIGAERU, OCEAN_PRINCE,
		RINGO, MAGURO, RISUKUMA, ECOLO, FELI, LEMRES, OSHARE_BONES, YU_REI,
	};
	sprite background = {};
	sprite holder[24] = {};
	sprite charSprite[24] = {};
	fimage* backgroundImage = nullptr;

	// Player stuff
	sprite* selectSprite = nullptr;
	sprite* selectedCharacter = nullptr;
	sprite* dropset = nullptr;
	sprite* name = nullptr;
	sprite* nameHolder = nullptr;
	sprite* nameHolderNumber = nullptr;
	sprite* playernumber = nullptr;
	int* sel = nullptr;
	bool* madeChoice = nullptr;
	float scale = 1.f;
	int Nplayers = 0;
	bool firstStart = true;
};

}
