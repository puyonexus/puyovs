#pragma once

#include "global.h"
#include "Sprite.h"

namespace ppvs
{

class Game;

class Menu final {
public:
    explicit Menu(Game*);
	~Menu();

	Menu(const Menu&) = delete;
	Menu& operator=(const Menu&) = delete;
	Menu(Menu&&) = delete;
	Menu& operator=(Menu&&) = delete;

	void draw();
	void play();
	int m_select;

private:
	Game* m_currentGame;
	GameData* m_data;
	Sprite m_background;
	Sprite m_option[3];
	bool m_disableRematch;
};

}
