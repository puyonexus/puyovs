#pragma once

#include "global.h"
#include "Sprite.h"

namespace ppvs
{

class Game;

class Menu
{
public:
	Menu(Game*);
	virtual ~Menu();

	void draw();
	void play();
	int select;

private:
	Game* currentgame;
	GameData* data;
	Sprite background;
	Sprite option[3];
	bool disableRematch;
};

}
