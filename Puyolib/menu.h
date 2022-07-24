#pragma once

#include "global.h"
#include "sprite.h"

namespace ppvs
{

class game;

class menu
{
public:
	menu(game*);
	virtual ~menu();

	void draw();
	void play();
	int select;

private:
	game* currentgame;
	gameData* data;
	sprite background;
	sprite option[3];
	bool disableRematch;
};

}
